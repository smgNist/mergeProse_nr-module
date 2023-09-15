import os
from multiprocessing import Pool
import subprocess

numerology = [2, 0]
mcs = [14]
enableSensing = ['false'] 
rri = [0]
maxNTx = [1, 4, 8]
harqFeedback = ['false'] 


def start_simulation(params):
    numerology, mcs, enableSensing, rri, maxNTx, harqFeedback = params
    outputDir = "output_slNum-%d_mcs-%d_sensing-%s_rri-%d_maxNTx-%d_harqF-%s" % (numerology, mcs, enableSensing, rri, maxNTx, harqFeedback)
    print(f'Output dir: %s'  % (outputDir))
    os.system('mkdir %s' % (outputDir))
    os.system('./ns3 run "nr-mcptt-operational-modes-static --minNumAccess=300 --RngRun=2 --showProgress=true --slNumerology=%d --slMcs=%d --enableSensing=%s --rri=%d --maxNTx=%d --harqFeedback=%s" --cwd="%s" > %s/ouput.txt 2>&1' % (numerology, mcs, enableSensing, rri, maxNTx, harqFeedback, outputDir, outputDir))
    os.system('python3 mcptt-operational-modes-plot-access-time.py %s/mcptt-access-time.txt' % (outputDir)) 
    os.system('python3 mcptt-operational-modes-plot-m2e-latency.py %s/mcptt-m2e-latency.txt' % (outputDir)) 


# Set the number of simultaneous processes
processes = 3

# Create a list of parameters for each run
params = [(n, m, s, r, ntx, h) for n in numerology for m in mcs for s in enableSensing for r in rri for ntx in maxNTx for h in harqFeedback]

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