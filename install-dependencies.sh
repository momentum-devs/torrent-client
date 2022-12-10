rm -rf externals/fmt/build
mkdir externals/fmt/build
cd externals/fmt/build || exit
cmake ..
sudo make install
cd ../../..

rm -rf externals/expected-lite/build
mkdir externals/expected-lite/build
cd externals/expected-lite/build || exit
cmake -DEXPECTED_LITE_OPT_BUILD_TESTS=OFF \
      -DEXPECTED_LITE_OPT_BUILD_EXAMPLES=OFF  ..
sudo make install
cd ../../..

rm -rf externals/gsl-lite/build
mkdir externals/gsl-lite/build
cd externals/gsl-lite/build || exit
cmake -DGSL_LITE_OPT_BUILD_TESTS=OFF \
      -DGSL_LITE_OPT_BUILD_CUDA_TESTS=OFF \
      -DGSL_LITE_OPT_BUILD_EXAMPLES=OFF  ..

sudo make install
cd ../../..

rm -rf externals/bencode/build
mkdir externals/bencode/build
cd externals/bencode/build || exit
cmake -DCMAKE_BUILD_TYPE=Release \
      -DBENCODE_BUILD_TESTS=OFF \
      -DBENCODE_BUILD_BENCHMARKS=OFF \
      -DBENCODE_BUILD_DOCS=OFF \
      ..
sudo make install
cd ../../..
