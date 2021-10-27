#!/bin/bash
rm -f ece3058_cachelab_extracredit_submission.tar.gz
tar -czvf ece3058_cachelab_extracredit_submission.tar.gz cachesim.c cachesim.h lrustack.c lrustack.h extra_tests.c trace_extra1.txt trace_extra2.txt trace_extra3.txt trace_extra4.txt trace_extra5.txt trace_stats.txt associativity_opt.txt trace_stats.sh associativity_opt.sh
echo "Done!"
echo "The files that will be submitted are:"
tar -ztvf ece3058_cachelab_extracredit_submission.tar.gz
