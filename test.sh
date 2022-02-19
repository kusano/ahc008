set -eu

g++ -O2 chase.cpp

pushd tools

for i in $(seq 0 99)
do
  name=$(printf %04d.txt ${i})
  echo ${name} 1>&2
  cargo run --release --bin tester ../a.out < in/${name} > out/${name}
done

popd
