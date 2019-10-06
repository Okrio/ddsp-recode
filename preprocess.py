import numpy as np
import soundfile as sf
import librosa as li
from tqdm import tqdm
import os
from hparams import preprocess

def multiScaleFFT(x, scales, overlap=75/100):
    stfts = []
    for scale in scales:
        stfts.append(abs(
            li.stft(x, n_fft=scale, hop_length=int((1-overlap)*scale), center=False)
        ))
    return stfts

def process(filename, block_size, sequence_size, fmin, fmax):
    os.makedirs("output", exist_ok=True)

    sound = sf.SoundFile(filename)
    batch = len(sound) // (block_size * sequence_size)

    scales = preprocess.fft_scales
    output = preprocess.output_dir
    sp = []
    for scale, ex_sp in zip(scales,multiScaleFFT(np.random.randn(block_size * sequence_size), scales)):
        sp.append(np.memmap(f"{output}/sp_{scale}.npy",
                            dtype=np.float32,
                            shape=(batch, ex_sp.shape[0], ex_sp.shape[1]),
                            mode="w+"))

    print(f"Splitting data into {batch} examples of {sequence_size}-deep sequences of {block_size} samples.")

    f0 = np.memmap(f"{output}/f0.npy", dtype="float32", shape=(batch,sequence_size), mode="w+")
    lo = np.memmap(f"{output}/lo.npy", dtype="float32", shape=(batch,sequence_size), mode="w+")

    fs = sound.samplerate
    tmin = fs // fmax
    tmax = fs // fmin

    for b in tqdm(range(batch)):

        x = sound.read(block_size * sequence_size)
        for i,msstft in enumerate(multiScaleFFT(x, scales)):
            sp[i][b,:,:] = msstft

        x = x.reshape(-1, block_size)
        for i,seq in enumerate(x):
            seq_win = seq * np.hanning(block_size)
            corr = np.correlate(seq_win,seq_win,"full")[block_size:]
            t0 = np.argmax(corr[tmin:tmax])+tmin

            f0[b,i] = fs / t0
            lo[b,i] = np.sqrt(np.mean(seq**2))


if __name__ == '__main__':
    process(preprocess.input_filename,
            preprocess.block_size,
            preprocess.sequence_size,
            preprocess.fmin,
            preprocess.fmax)
