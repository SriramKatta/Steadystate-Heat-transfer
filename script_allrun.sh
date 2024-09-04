#!/bin/bash -l

job1id=$(sbatch  script_1ccnuma.sh | awk '{print $(NF)}')
job2id=$(sbatch -d after:$job1id+1minutes  script_4ccnuma.sh | awk '{print $(NF)}')
job3id=$(sbatch -d after:$job2id+1minutes   script_codebalance_1ccnuma.sh | awk '{print $(NF)}')
