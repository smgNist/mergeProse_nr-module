import os
from multiprocessing import Pool
import subprocess

numerology = [0, 2]
mcs = [14]
enableSensing = ['false', 'true'] 
rri = [0]
maxNTx = [1, 4, 8]
harqFeedback = ['false'] 
offNetMediaSps = ['false', 'true']
relayMediaSps = ['true', 'false']

def start_simulation(params):
    numerology, mcs, enableSensing, rri, maxNTx, harqFeedback, offNetMediaSps, relayMediaSps = params
    outputDir = "output_slNum-%d_mcs-%d_sensing-%s_rri-%d_maxNTx-%d_harqF-%s_offNetMediaSps-%s_relayMediaSps-%s" % (numerology, mcs, enableSensing, rri, maxNTx, harqFeedback, offNetMediaSps, relayMediaSps)
    print(f'Output dir: %s'  % (outputDir))
    os.system('mkdir %s' % (outputDir))
    os.system('./ns3 run "nr-mcptt-operational-modes-static --minNumAccess=1000 --RngRun=2 --showProgress=true --slNumerology=%d --slMcs=%d --enableSensing=%s --rri=%d --maxNTx=%d --harqFeedback=%s --offNetMediaSps=%s --relayMediaSps=%s" --cwd="%s" > %s/ouput.txt 2>&1' % (numerology, mcs, enableSensing, rri, maxNTx, harqFeedback, offNetMediaSps, relayMediaSps, outputDir, outputDir))
    os.system('python3 mcptt-operational-modes-plot-access-time.py %s/mcptt-access-time.txt' % (outputDir)) 
    os.system('python3 mcptt-operational-modes-plot-m2e-latency.py %s/mcptt-m2e-latency.txt' % (outputDir)) 


# Set the number of simultaneous processes
processes = 12

# Create a list of parameters for each run
params = [(n, m, s, r, ntx, h, oms, rms) for n in numerology for m in mcs for s in enableSensing for r in rri for ntx in maxNTx for h in harqFeedback for oms in offNetMediaSps for rms in relayMediaSps]

# Print the total number of simulations
print(f'Total number of simulations: {len(params)}')

# Create a Pool object with "processes" number of processes
pool = Pool(processes=processes)

# Start the simulations
os.system('./ns3')
for _ in pool.imap_unordered(start_simulation, params):
    pass
pool.close()
pool.join()