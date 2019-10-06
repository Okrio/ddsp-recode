class preprocess:
    input_filename = "flute_16.wav" # must be a mono, 44100Hz .wav file
    output_dir     = "output"
    crepe_f0       = f"{output_dir}/flute.f0.csv"

    fft_scales     = [2048, 1024, 512, 256, 128, 64] # Multi scale stft objective
    block_size     = 160 # Must be the same block size than that of crepe !!
    sequence_size  = 250 # Number of sequence to process in the GRU cell
    num_batch      = 298 # Must be changed after preprocessing......
