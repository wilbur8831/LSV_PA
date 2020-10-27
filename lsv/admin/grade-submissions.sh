#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <ALL|branch-name>"
    echo "ALL: grade ALL students"
    echo "branch-name: the branch to grade, e.g., d04943019"
    exit 1
fi

ref_prog="pa1-ref"
pa_cmd="lsv_print_sopunate"
pa_dir="lsv/pa1"
ref_dir="${pa_dir}/ref"
out_dir="${pa_dir}/out"
diff_dir="${pa_dir}/diff"
bench_list=( $(find -L benchmarks/best_results/ -type f -name '*.blif') )
students=(master d04943019 b05901015-back)
#students=( $(cut -d, -f1 < lsv/admin/participants-id.csv | tail -n +3) )

grade_one_branch () {
    student="$1"
    result="${pa_dir}/${student}.csv"
    git switch "${student}"
    make -j8
    echo "Benchmark,Result" > "${result}"
    rm -rf "${ref_dir}" "${out_dir}" "${diff_dir}"
    mkdir -p "${ref_dir}"
    mkdir -p "${out_dir}"
    mkdir -p "${diff_dir}"
    declare -i correct=0
    for bench in "${bench_list[@]}"; do
        echo "[INFO] Testing with ${bench} ..."
        tcl="read ${bench}; ${pa_cmd}"
        bch_name=$(echo "${bench}" | awk -F "/" '{print $(NF)}' | sed -e 's/.blif$//')
        ./"${ref_prog}" -c "${tcl}" > "${ref_dir}/${bch_name}.txt"
        ./abc -c "${tcl}" > "${out_dir}/${bch_name}.txt"
        diff -uwiB "${ref_dir}/${bch_name}.txt" "${out_dir}/${bch_name}.txt" > "${diff_dir}/${bch_name}.txt"
        if [ "$?" -eq 0 ]; then
            ((++correct))
            echo "${bench},Pass" >> "${result}"
        else
            echo "${bench},Fail" >> "${result}"
        fi
    done
    local __return_var="$2"
    local __point=$(echo "${correct}*2" | bc)
    eval "${__return_var}"="${__point}"
    echo "[INFO] Correct cases: ${correct}"
    echo "[INFO] Total points: ${__point}"
    echo "Score,${__point}" >> "${result}"
}

if [ "$1" = "ALL" ]; then
    echo "[INFO] Grading all students ..."
    student_points=()
    for student in "${students[@]}"; do
        grade_one_branch "${student}" point
        student_points+=("${point}")
        git add "${pa_dir}/${student}.csv"
        git add "${ref_dir}" "${out_dir}" "${diff_dir}"
        git commit -m "Grade branch ${student}"
        #git push
    done
    git sw master
    echo "${student_points[@]}"
else
    echo "[INFO] Grading branch $1 ..."
    grade_one_branch "$1" point
fi
