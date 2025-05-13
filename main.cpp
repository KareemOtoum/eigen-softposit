
#include "softposit_cpp.h"
#include <Eigen/Dense>
#include <chrono>

namespace Eigen 
{
    template<>
    struct NumTraits<posit16> {
        using Self = posit16;
        using Real = posit16;
        using NonInteger = posit16;
        using Nested = posit16;
        using Literal = float;
    
        enum {
            IsComplex = 0,
            IsInteger = 0,
            IsSigned = 1,
            RequireInitialization = 1,
            ReadCost = 1,
            AddCost = 2,
            MulCost = 2
        };
    
        static inline Real epsilon() { return p16(0.00001f); }
        static inline Real dummy_precision() { return p16(0.00001f); }
        static inline int digits10() { return 3; }  // arbitrary safe num
    };
}

void benchmark(int r, int c, int repetitions)
{
    assert(repetitions > 0);
    using namespace std::chrono;
    Eigen::Matrix<posit16, Eigen::Dynamic, Eigen::Dynamic> pa(r, c);
    Eigen::Matrix<posit16, Eigen::Dynamic, Eigen::Dynamic> pb(r, c);

    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> fa(r, c);
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> fb(r, c);

    pa.fill(p16(5.241));
    pb.fill(p16(3.333));
    fa.fill(float(5.241));
    fb.fill(float(3.333));

    duration<double, std::micro> pelapsed; 
    duration<double, std::micro> felapsed;
    double mean_error{};
    for(int i {}; i < repetitions; ++i)
    {
        auto pstart = high_resolution_clock::now();
        auto pmul = pa * pb;
        volatile auto padd = pa + pb;
        volatile auto pmin = pa - pb;
        auto pend = high_resolution_clock::now();
        pelapsed += pend - pstart;

        auto fstart = high_resolution_clock::now();
        auto fmul = fa * fb;
        volatile auto fadd = fa + fb;
        volatile auto fmin = fa - fb;
        auto fend = high_resolution_clock::now();
        felapsed += fend - fstart;
        
        // calculate error
        Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic> 
            p_to_f(pmul.rows(), pmul.cols());

        for(int row{}; row < pmul.rows(); ++row) {
            for(int col{}; col < pmul.cols(); ++col) {
                p_to_f(row, col) = pmul(row, col).toDouble();
            }
        }
        
        if (!fmul.allFinite()) {
            std::cerr << "Float result has NaN or Inf at size " << r << "x" << c << "\n";
            return;
        }
        if (!pa.allFinite() || !pb.allFinite()) {
            std::cerr << "Posit input matrices are invalid at size " << r << "x" << c << "\n";
            return;
        }

        auto abs_error = (fmul - p_to_f).cwiseAbs();
        mean_error += abs_error.mean();
    }
    pelapsed /= repetitions;
    felapsed /= repetitions;
    mean_error /= repetitions;

    std::cout << "\t--------Matrix Size: " << 
    r << "x" << c << "--------\n";
    std::cout << "\t Posit Time taken: " << pelapsed.count() << "\n";
    std::cout << "\t Float Time taken: " << felapsed.count() << "\n";
    std::cout << "\t Posit Mean Absolute Error: " << mean_error << "\n";
}

int main()
{
    #ifdef EIGEN_VECTORIZE_SSE
        std::cout << "SSE enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_AVX
        std::cout << "AVX enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_AVX512
        std::cout << "AVX-512 enabled\n";
    #endif
    #ifdef EIGEN_VECTORIZE_NEON
        std::cout << "NEON enabled (ARM)\n";
    #endif
    #ifndef EIGEN_VECTORIZE
        std::cout << "No SIMD vectorization\n";
    #endif

    for(int i{ 10 }; i <= 50; i += 10)
    {
        benchmark(i, i, 2);
    }
      
    return 0;
}