# %%
from time import sleep
from serial import Serial
import numpy as np
import pandas as pd

from matplotlib import pyplot as plt
import seaborn as sns


def printRawODs(serialDevice, ledTotTime):
    serialDevice.write("0".encode("ascii"))
    sleep(ledTotTime)
    serialDevice.timeout = 0.2
    for idx, line in enumerate(serialDevice):
        print("{}, ".format(idx) + line.decode("ascii").strip())


def readBlank(serialDevice: Serial, replicates: int, ledTotTime: float):
    """Generates replicated data for a blank read and returns as a flat numpy array

    Args:
        serialDevice (Serial): Serial device doing the reads
        replicates (int): Nummber of replicates for each well
        ledTotTime (float): Time to wait before reading the serial bus
    """

    # Flat python-side conversion data buffer
    raws = np.zeros(shape=(replicates, 96))

    serialDevice.timeout = 0.4

    for rep in range(replicates):
        serialDevice.write("A".encode("ascii"))
        sleep(ledTotTime)  # change based on LED ontime

        for well, reading in enumerate(serialDevice):
            raws[rep][well] = int(reading.decode("ascii").strip())
        print(raws[rep])

    return raws


def readODSDprop(
    serialDevice: Serial,
    replicates: int,
    blankReps: np.ndarray,
    ledTotTime: float,
):
    """Collects raw reads from the device, determines mean and SD,
    then converts to optical density and propagated SD.

    Args:
        serialDevice (Serial): Serial device doing the reads.
        replicates (int): Number of replicates for each well to be averaged.
        blankReps (np.ndarray): numpy array of shape (replicates, 96) with the blank reads.
        ledTotTime (float): Time in seconds to wait for device to finish well reads

    Returns:
        np.array: Flat array with well OD means
        np.array: Flat array with well OD propagated SD
    """

    raws = np.zeros(shape=(replicates, 96))
    serialDevice.timeout = 0.2

    for rep in range(replicates):
        serialDevice.write("A".encode("ascii"))
        sleep(ledTotTime)  # change based on LED ontime

        for well, reading in enumerate(serialDevice):
            raws[rep][well] = int(reading.decode("ascii").strip())

    # calculate means and SDs between the read replicates
    rawMeans = np.mean(raws, axis=0, dtype=np.float64)
    rawSDs = np.std(raws, axis=0, dtype=np.float64)

    blankMeans = np.mean(blankReps, axis=0, dtype=np.float64)
    blankSDs = np.std(blankReps, axis=0, dtype=np.float64)

    q = np.zeros(shape=(replicates, 96))  # calculation buffer
    K = 1.2075  # logarithmic output slope per decade
    Iref = 100e-9  # reference current with 500k Rset

    # Logarithmic factor from the logamp for the photodiode incident
    # light intensity compared to the arbitrary reference current.
    # The SD determination is the same since this is a constant multiplier
    decFac = lambda ADCMeans: ((ADCMeans * 5 / 4096) / K)

    ODMeans = decFac(blankMeans) - decFac(rawMeans)
    ODSDs = decFac(blankSDs) + decFac(rawSDs)

    return (ODMeans, ODSDs)


def readODSDrand(
    serialDevice: Serial,
    replicates: int,
    blankReps: np.ndarray,
    ledTotTime: float,
):
    """Collects raw reads and blanks from the device, converts to optical density
     for each replicate, and then determines means and SDs.

    Args:
        serialDevice (Serial): Serial device doing the reads.
        blankReps (np.ndarray): Flat array with the blank ADC convertion means.
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

    # calculate means and SDs
    rawMeans = np.mean(raws, axis=0, dtype=np.float64)
    rawSDs = np.std(raws, axis=0, dtype=np.float64)

    q = np.zeros(shape=(replicates, 96))  # calculation buffer
    K = 1.2075  # logarithmic output slope per decade
    Iref = 100e-9  # reference current with 500k Rset

    # logarithmic factor for photodiode incident light intensity compared to arbitrary reference current
    decFacMean = lambda ADCMeans: ((ADCMeans * 5 / 4096) / K)

    q = decFacMean(blankReps) - decFacMean(raws)

    # Do the mean and SD calculations on the multiple blank reads
    ODMeans = np.mean(q, axis=0, dtype=np.float64)
    ODSDs = np.std(q, axis=0, dtype=np.float64)

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
# Set > (ledOn + deadTime) * 96
ledTotTime = 1.2

opspec = Serial(arduino_port, baud)
sleep(2)  # Important to let serial start running


# %% Test device
# opspec.write("T".encode("ascii"))
# opspec.write("A".encode("ascii"))
printRawODs(opspec, ledTotTime)

# %%

rep = 10
blank = readBlank(opspec, rep, ledTotTime)

# %%
OD, ODSD = readODSDrand(opspec, rep, blank, ledTotTime)
# OD, ODSD = readODSDprop(opspec, rep, blank, ledTotTime)

# %%
fig, axs = plt.subplots(2)

sns.heatmap(np.abs(inflateODArray(OD).T), ax=axs[0], linewidth=0.5)
axs[0].set_title("Test Plate Absolute Mean OD (n = {})".format(rep))

sns.heatmap(inflateODArray(ODSD).T, ax=axs[1], linewidth=0.5)
axs[1].set_title("Well OD Standard deviation")

fig.tight_layout()

plt.show()

# %%
img_format = "svg"
fname = "test_data/testplate.{}".format(img_format)
fig.savefig(fname, format=img_format, dpi=1200)

# %%
opspec.close()
