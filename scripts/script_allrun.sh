#!/bin/bash -l

job1id=$(sbatch  script_codebalance_1ccnuma.sh | awk '{print $(NF)}')
job2id=$(sbatch -d afterok:$job1id  script_1ccnuma.sh | awk '{print $(NF)}')
job3id=$(sbatch -d afterok:$job2id  script_4ccnuma.sh | awk '{print $(NF)}')
