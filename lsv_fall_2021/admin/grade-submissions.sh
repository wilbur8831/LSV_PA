#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <ALL|branch-name>"
    echo "ALL: grade ALL students"
    echo "branch-name: the branch to grade, e.g., d04943019"
    exit 1
fi

pa_cmd="lsv_print_msfc"
pa_dir="lsv_fall_2021/pa1"
ref_dir="${pa_dir}/ref"
out_dir="${pa_dir}/out"
diff_dir="${pa_dir}/diff"
bench_dir="benchmarks"
bench_list=( $(find -L benchmarks/arithmetic/ -type f -name '*.aig') 
             $(find -L benchmarks/random_control/ -type f -name '*.aig') )
students=( $(cut -d, -f1 < lsv_fall_2021/admin/participants-id.csv | tail -n +3) )

grade_one_branch () {
    student="$1"
    result="${pa_dir}/${student}.csv"
    git switch "${student}"
    for extdir in src/ext*; do
        rm ${extdir}/*.o ${extdir}/*.d
    done
    rm src/base/abci/*.o src/base/abci/*.d
    make -j8
    echo "Benchmark,Result" > "${result}"
    rm -rf "${out_dir}" "${diff_dir}"
    mkdir -p "${out_dir}"
    mkdir -p "${diff_dir}"

    declare -i blif_eq=0 
    for blif in ${pa_dir}/*.blif; do 
        echo "[INFO] Testing with ${blif} ..."
        ./abc -c "dsec -n ${ref_dir}/sa4_ref.blif ${blif}" > "${out_dir}/sa4.txt" 
        match=$(grep -o -c "Networks are equivalent" ${out_dir}/sa4.txt )  
        if [ ${match} -eq 1 ]; then
                ((++blif_eq))
            echo "${blif},Pass" >> "${result}"
        else
            echo "${blif},Fail" >> "${result}"
        fi 
    done
    declare -i correct=0
    for bench in "${bench_list[@]}"; do
        echo "[INFO] Testing with ${bench} ..."
        bch_name=$(echo "${bench}" | awk -F "/" '{print $(NF)}' | sed -e 's/.aig$//')
        ./abc -c "${pa_cmd}" "${bench}" > "${out_dir}/${bch_name}.txt"
        diff -uwiB "${ref_dir}/${bch_name}.txt" "${out_dir}/${bch_name}.txt" > "${diff_dir}/${bch_name}.txt"
        if [ "$?" -eq 0 ]; then
            ((++correct))
            echo "${bench},Pass" >> "${result}"
        else
            echo "${bench},Fail" >> "${result}"
        fi
    done
    local __return_var="$2"
    local __point=$(echo "${correct}*4+${blif_eq}*5" | bc)
    eval "${__return_var}"="${__point}"
    echo "[INFO] Correct BLIF  in part 1: ${blif_eq}"
    echo "[INFO] Correct cases in part 3: ${correct}"
    echo "[INFO] Total points: ${__point}"
    echo "Score,${__point}" >> "${result}"
}

if [ "$1" = "ALL" ]; then
    echo "[INFO] Grading all students ..."
    student_points=()
    for student in "${students[@]}"; do
        grade_one_branch "${student}" point
        student_points+=("${point}")
        git add -f "${pa_dir}/${student}.csv"
        git add -f "${out_dir}" "${diff_dir}"
        git commit -m "Grade branch ${student}"
        git push
        git stash push -m "Garbage from ${student}"
    done
    git switch master
    all_result="${pa_dir}/ALL.csv"
    echo "Student,Points" > "${all_result}"
    for i in "${!students[@]}"; do
        echo "${students[$i]},${student_points[$i]}" >> "${all_result}"
    done
    git add "${all_result}"
    git commit -m "Grade the PAs of students"
    git push
else
    echo "[INFO] Grading branch $1 ..."
    grade_one_branch "$1" point
fi
