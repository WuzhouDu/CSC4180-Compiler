#!bin/bash

# Step 1: 编译
make all

# 检查编译是否成功
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# 初始化测试结果比较变量
result="all right!"

# 循环执行测试并比较结果
for ((i=0; i<=5; i++)); do
    # Step 2: 运行 scanner
    ./scanner ../testcases/test$i.oat

    # Step 3: 运行 lexer
    ./lexer < ../testcases/test$i.oat

    # Step 4: 比较输出结果
    diff_result=$(diff ./scannerOutput.txt ./lexOutput.txt)

    # 检查输出是否相同
    if [ "$diff_result" != "" ]; then
        echo "Test case $i failed: Output differs."
        result="Test case $i failed."
        break
    fi

    echo "Test case $i success."
done

# Step 5: 输出测试结果
echo $result