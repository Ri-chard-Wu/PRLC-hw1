cd ~/hw1
make clean
make 2>make-stderr.out


echo "==================================="
echo "=           Validate all          ="
echo "==================================="

echo "" 1> run-stderr.out

for i in {1..4} {7..9}
do
  srun -n 1 -c 1 ./hw1 ./samples/0${i}.txt 2>> run-stderr.out 1> answer.txt
  echo -n "0"${i}".txt: "
  ./validate.py ./samples/0${i}.txt answer.txt 2>> run-stderr.out 
done

for i in {10..19}
do
  srun -n 1 -c 1 ./hw1 ./samples/${i}.txt 2>> run-stderr.out 1> answer.txt
  echo -n ${i}".txt: "
  ./validate.py ./samples/${i}.txt answer.txt 2>> run-stderr.out
done


# echo "==================================="
# echo "=      Print make-stderr.out      ="
# echo "==================================="

# cat make-stderr.out

# echo "==================================="
# echo "=      Print run-stderr.out       ="
# echo "==================================="

# cat run-stderr.out




echo ""
echo ""