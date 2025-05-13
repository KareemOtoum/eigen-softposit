
# Benchmarking SoftPosit in Eigen: Evaluating Posit16 vs Float in Matrix Arithmetic  

### The goal of this assignment is to identify a C/C++ library that could benefit from posit number representations over IEEE-754 floating point, implementing posit support using the softposit-cpp library and evaluate performance and numerical accuracy

IEEE-754 floats have well known accuracy issues (rounding, underflow/overflow)

Posits Offer:
 - Higher accuracy near 1.0
 - Graceful underflow
 - Better dynamic range in fewer bits

Softposit is a software implementation of the posit standard therefore it can be slower than hardware floats due to optimizations,
but may offer better accuracy or compactness