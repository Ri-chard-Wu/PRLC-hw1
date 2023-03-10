
cd ~/hw1
# make clean
# make 2>make-stderr.out
rm hw1
g++ -std=c++17 -O3 -pthread -fopenmp hw1.cc -o hw1

echo "==================================="
echo "=             Run hw1             ="
echo "==================================="

./hw1 ./samples/${1}.txt 2> run-stderr.out

echo "==================================="
echo "=      Print make-stderr.out      ="
echo "==================================="

cat make-stderr.out

echo "==================================="
echo "=      Print run-stderr.out       ="
echo "==================================="

cat run-stderr.out



echo ""
echo ""


