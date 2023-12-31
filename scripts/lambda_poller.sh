#!/bin/sh
cd ${WORKSPACE}/dataset/lambda-client
python3 lambdad.py &

cd ${WORKSPACE}
./scripts/python/lambda_poller.py \
--engine_name_file_paths mining_engine_names.txt \
--puzzle_solver_file_path src/solver \
--jobs `grep processor /proc/cpuinfo | wc -l` \
--engine_file_path src/solver
