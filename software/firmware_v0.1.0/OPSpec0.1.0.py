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
        replicates (int): Nummber of replicates for each well
    """

    # Flat python-side conversion data buffer
    raws = np.zeros(shape=(replicates, 96))

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


# Update
def readOD(
    serialDevice: Serial,
    blankMeans: np.ndarray,
    ledTotTime: float,
    replicates: int = 5,
):
    """Collects raw reads from the device and converts to optical density.

    Args:
        serialDevice (Serial): Serial device doing the reads.
        blankMeans (np.ndarray): Flat array with the blank ADC convertions.
        replicates (int): Number of replicates for each well to be averaged. Defaults to 5.
        ledTotTime (float): Time in seconds to wait for device to finish well reads

    Returns:
        np.array: Flat array with well ODs
    """

    raws = np.zeros(shape=(replicates, 96))

    serialDevice.timeout = 0.2

    for rep in range(replicates):
        serialDevice.write("A".encode("ascii"))
        sleep(ledTotTime)  # change based on LED ontime

        for well, reading in enumerate(serialDevice):
            raws[rep][well] = int(reading.decode("ascii").strip())

    # # Do the mean and SD calculations on the multiple blank reads

    # Vf = schottkyDrop * 4096
    q = np.zeros(shape=(replicates, 96))  # calculation buffer
    K = 1.2075  # logarithmic output slope per decade
    Iref = 100e-9  # reference current with 500k Rset

    Iread = lambda ADCraw: np.math.exp((ADCraw * 5 / 4096) / K) * Iref

    for i, rep in enumerate(q):
        q[i] = Iread(blankMeans) / Iread(raws[i])

    flatOD = log10(q)

    ODMeans = np.mean(flatOD, axis=0, dtype=np.float64)
    ODSDs = np.std(flatOD, axis=0, dtype=np.float64)

    return (ODMeans, ODSDs)


def inflateODArray(flatOD):
    """Inflates a flat data buffer into the 2-dimensional array indexed by well location.

    Args:
        flatOD (np.ndarray): 1D numpy array with well data grouped by column

    Returns:
        np.ndarray: 2D numpy array of reads indexed by [column][row] well locations
    """

    ODs = np.zeros(shape=(12, 8))
    for i in range(len(flatOD)):
        ODs[i // 8][i % 8] = flatOD[i]

    return ODs


# %%

# Setup arduino
arduino_port = "COM3"
baud = 115200
ledTotTime = 0.9


opspec = Serial(arduino_port, baud)
sleep(1.5)  # Important to let serial start running

# %% Test device
opspec.write("T".encode("ascii"))

# %%
blanks, blankSD = readBlank(opspec, 10, ledTotTime)

# %%

ODdf = pd.DataFrame()

ODs, ODSDs = readOD(opspec, blanks, ledTotTime, 5)

# %%
opspec.close()
