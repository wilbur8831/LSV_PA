students=( $(cut -d, -f1 < lsv/admin/participants-id.csv | tail -n +3) )
for student in "${students[@]}"; do
    echo "Updating branch ${student} ..."
    git sw "${student}"
    git pull
done
