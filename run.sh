
cd ~/hw1

# make clean
# make 2>make-stderr.out
rm ${2}
g++ -std=c++17 -O3 -pthread -fopenmp ${2}.cc -o ${2} 2>make-stderr.out

echo "==================================="
echo "=             Run hw1             ="
echo "==================================="

srun -n 1 -c 5 ./${2} ./samples/${1}.txt 2> run-stderr.out 1> answer.txt


echo "==================================="
echo "=      Print make-stderr.out      ="
echo "==================================="

cat make-stderr.out

echo "==================================="
echo "=      Print run-stderr.out       ="
echo "==================================="

cat run-stderr.out

echo "==================================="
echo "=            Validate             ="
echo "==================================="

./validate.py ./samples/${1}.txt answer.txt


echo ""
echo ""


