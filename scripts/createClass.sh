# TODO script that creates a .cpp and corresponding .hpp file with relevant boilerplate. Should accept a path and a class name as arguments

targetDirectory="$1"
className="$2"
fileName="$3"

cppFile="$targetDirectory/$fileName.cpp"
hppFile="$targetDirectory/$fileName.hpp"

cat ./scripts/template.hpp | sed -e "s/CLASS_NAME/$className/g" >> $hppFile
cat ./scripts/template.cpp | sed -e "s/CLASS_NAME/$className/g" | sed -e "s/HEADER_FILE/$fileName.hpp/g" >> $cppFile
