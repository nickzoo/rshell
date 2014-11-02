echo "#ls no arguments"
echo "bin/ls"
bin/ls
echo ----------------

echo "#ls -a hidden files"
echo "bin/ls -a"
bin/ls -a
echo ----------------

echo "#ls -l long format"
echo "bin/ls -l"
bin/ls -l
echo ----------------

echo "#ls -R recursive"
echo "bin/ls -R"
bin/ls -R
echo ----------------

echo "#combinations"
echo "bin/ls -a -l"
bin/ls -a -l
echo ----------------

echo "bin/ls -a -R"
bin/ls -a -R
echo ----------------
    
echo "bin/ls -l -R"
bin/ls -l -R
echo ----------------

echo "#various input formats"
echo "bin/ls -al"
bin/ls -al
echo ----------------

echo "bin/ls -la"
bin/ls -la
echo ----------------

echo "bin/ls -l -a"
bin/ls -l -a
echo ----------------

echo "#file arguments"
echo "bin/ls Makefile README LICENSE ls.sh"
bin/ls Makefile README LICENSE ls.sh
echo ----------------

echo "#directory arguments"
echo "bin/ls bin src tests"
bin/ls bin src tests
echo ----------------

echo "#file and directory arguments"
echo "bin/ls Makefile bin README src tests LICENSE ls.sh"
bin/ls Makefile bin README src tests LICENSE ls.sh
echo ----------------

echo "#nonexistent file"
echo "bin/ls foo"
bin/ls foo
echo ----------------

echo "#nonexistent and existent file together"
echo "bin/ls foo src"
bin/ls foo src
echo ----------------

echo "#illegal argument"
echo "ls -0"
bin/ls -0
echo ----------------

echo "#illegal argument and file"
echo "ls -0 README"
bin/ls -0 README
echo ----------------

echo "#ordering matters"
echo "ls README -0"
bin/ls README -0
echo ----------------
