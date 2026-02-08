searchDirectory="./src"
searchPattern="$1"

readarray -t grepResult < <(grep -rnw "$searchDirectory" -e "$searchPattern")
declare grepResult


declare -A fileNames

for line in "${grepResult[@]}"
do
    
    fileName="$(echo "$line" | cut -d: -f1)"
    lineNumber="$(echo "$line" | cut -d: -f2)"
    lineContents="$(echo "$line" | cut -d: -f3)"
    
    if [[ -v fileNames["$fileName"] ]]
    then
        echo "  $lineNumber $lineContents"    
    else
        echo "$fileName:"
        echo "  $lineNumber $lineContents"
        fileNames["$fileName"]="Found!"
    fi
done
