#!/bin/bash

#ref_cmd="ps"
ref_cmd="print_unate -v"
pa2_cmd="lsv_print_pounate"
time_limit="900s"
pa_dir="lsv/pa2"
ref_dir="${pa_dir}/ref"
out_dir="${pa_dir}/out"
diff_dir="${pa_dir}/diff"
bench_list=( $(find -L benchmarks/ -maxdepth 2 -type f -name '*.aig') )

grade_one_branch () {
    student="$1"
    ref_result="${pa_dir}/ref.csv"
    pa2_result="${pa_dir}/${student}.csv"
    rm src/ext-lsv/*.o src/ext-lsv/*.d
    make -j8
    echo "Benchmark,Result" > "${ref_result}"
    echo "Benchmark,Result" > "${pa2_result}"
    rm -rf "${ref_dir}" "${out_dir}" "${diff_dir}"
    mkdir -p "${ref_dir}"
    mkdir -p "${out_dir}"
    mkdir -p "${diff_dir}"
    declare -i ref_correct=0
    declare -i pa2_correct=0
    for bench in "${bench_list[@]}"; do
        echo "[INFO] Testing with ${bench} ..."
        ref_tcl="read ${bench}; strash; ${ref_cmd}"
        pa2_tcl="read ${bench}; strash; ${pa2_cmd}"
        bch_name=$(echo "${bench}" | awk -F "/" '{print $(NF)}' | sed -e 's/.aig$//')
        timeout "${time_limit}" ./abc -c "${ref_tcl}" > "${ref_dir}/${bch_name}.txt"
        if [ "$?" -eq 0 ]; then
            ((++ref_correct))
            echo "${bench},Pass" >> "${ref_result}"
        else
            echo "${bench},Fail" >> "${ref_result}"
        fi
        timeout "${time_limit}" ./abc -c "${pa2_tcl}" > "${out_dir}/${bch_name}.txt"
        if [ "$?" -eq 0 ]; then
            ((++pa2_correct))
            echo "${bench},Pass" >> "${pa2_result}"
        else
            echo "${bench},Fail" >> "${pa2_result}"
        fi
    done
    echo "[INFO] REF correct cases: ${ref_correct}"
    echo "[INFO] PA2 correct cases: ${pa2_correct}"
    echo "Correct,${ref_correct}" >> "${ref_result}"
    echo "Correct,${pa2_correct}" >> "${pa2_result}"
}

echo "[INFO] Grading branch $1 ..."
grade_one_branch "$1"
