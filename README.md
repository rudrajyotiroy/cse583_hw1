# CSE 583 Winter 2025 Homework 1
CSE583 HW1 for statistical profiling of runtime execution of 4 C benchmarks

## Instructions:
./compile.sh to do everything

For graphs, change PASS=hw1 to PASS=dot-cfg in run.sh, then run ./compile,sh, then run 
find . -name "*.dot" -type f -exec sh -c 'dot -Tpdf "$0" > "${0%.dot}.pdf"' {} \;

