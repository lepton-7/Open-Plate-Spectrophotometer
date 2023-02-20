# %%
from time import sleep
from serial import Serial
import numpy as np
from numpy import log10
import pandas as pd


def printRawODs(serialDevice, ledTotTime):
    serialDevice.write("A".encode("ascii"))
    sleep(ledTotTime)
    serialDevice.timeout = 0.2
    for idx, line in enumerate(serialDevice):
        print("{}, ".format(idx) + line.decode("ascii").strip())


def readBlank(serialDevice: Serial, replicates: int, ledTotTime: float):
    """Generates data for a blank and returns as flat numpy arrays

    Args:
        serialDevice (Serial): Serial device doing the reads
        replicates (int): Nummber of replicates for each well to be averaged
    """

    # Python-side conversion data buffer
    raws = np.zeros(shape=(replicates, 16))

    serialDevice.timeout = 0.2

    for rep in range(replicates):
        serialDevice.write("A".encode("ascii"))
        sleep(ledTotTime)  # change based on LED ontime

        for well, reading in enumerate(serialDevice):
            raws[rep][well] = int(reading.decode("ascii").strip())

    # Do the mean and SD calculations on the multiple blank reads
    blankMeans = np.mean(raws, axis=0, dtype=np.float64)
    blankSDs = np.std(raws, axis=0, dtype=np.float64)

    return (blankMeans, blankSDs)


def readOD(
    serialDevice: Serial,
    blankMeans: np.ndarray,
    ledTotTime: float,
    schottkyDrop: float,
    replicates: int = 5,
):
    """_summary_

    Args:
        serialDevice (Serial): Serial device doing the reads
        blankMeans (np.ndarray): Flat array with the blank ADC convertions
        replicates (int): Number of replicates for each well to be averaged
        ledTotTime (float):
        schottkyDrop (float): Voltage drop of the Schottky between the OpAmp output and ADC

    Returns:
        np.array: Flat array with the well ODs
    """

    raws = np.zeros(shape=(replicates, 16))

    serialDevice.timeout = 0.2

    for rep in range(replicates):
        serialDevice.write("A".encode("ascii"))
        sleep(ledTotTime)  # change based on LED ontime

        for well, reading in enumerate(serialDevice):
            raws[rep][well] = int(reading.decode("ascii").strip())

    # Do the mean and SD calculations on the multiple blank reads

    Vf = schottkyDrop * 4096
    q = np.zeros(shape=(replicates, 16))

    for i, rep in enumerate(q):
        q[i] = (5 * blankMeans + Vf) / (5 * raws[i] + Vf)

    flatOD = log10(q)

    ODMeans = np.mean(flatOD, axis=0, dtype=np.float64)
    ODSDs = np.std(flatOD, axis=0, dtype=np.float64)

    return (ODMeans, ODSDs)


def inflateODArray(flatOD):

    ODs = np.zeros(shape=(2, 8))
    for i in range(len(flatOD)):
        ODs[i // 8][i % 8] = flatOD[i]

    return ODs


# %%

# Setup arduino
arduino_port = "COM5"
baud = 115200
ledTotTime = 0.9


opspec = Serial(arduino_port, baud)
sleep(1.5)  # Important to let serial start running

# %%
blanks, blankSD = readBlank(opspec, 10, ledTotTime)

# %%

ODdf = pd.DataFrame()

ODs, ODSDs = readOD(opspec, blanks, ledTotTime, 0.1, 5)

# %%
opspec.close()
