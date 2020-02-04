#!/bin/bash
pushd
cd src
for file in $(ls); do
    new_f=${file}_new
    clang-format $file > $new_f
    echo $file
    echo $new_f
    mv $new_f $file
done
popd
