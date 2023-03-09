#!/bin/sh

cd ~/hw1

echo *.o > .gitignore
# echo "first arg: ${1}"

git add *
git commit -m "${1}"
git push -u origin master