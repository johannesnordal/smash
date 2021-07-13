#include "Dist.hpp"
#include <vector>

double computeDist(double k, double Jaccard) {
    // Johannes: I added this so dist isn't inf.
    // If that was the intention, please feel free
    // to change it back.
    if (Jaccard == 0.0)
        return 1.0;

    return(-(1/k)*log(2*Jaccard/(1+Jaccard)));
}

double randomOccurance(int k, int n) {
    return(n/(n+pow(4,k)));
}

double computeRValue(int k, int n1, int n2) {
    double upper = randomOccurance(k,n1)*randomOccurance(k,n2);
    double lower = randomOccurance(k,n1) + randomOccurance(k,n2) - upper;
    return(upper/lower);
}

double initializeA(double rValue, int sketchSize, double rRatio, int sharedK) {
    double bx = pow(1-rValue, sketchSize); // Þetta er a_0
    for (int i = 0; i < sharedK; i++) {
        // Framreiknum upp í a_x, x er stærð sniðmengis
        bx *= (sketchSize - i)/(i+1)*rRatio;
    }
    return(bx);
}

int getThreshold(double rValue, int sketchSize) {
    int d = 20; // <=5% munur þegar við hættum
    double upper = d*rValue*sketchSize - 1 + rValue;
    double lower = d*rValue - rValue + 1;
    return(ceil(upper/lower));
}

double sumRatio(int sketchSize, double rRatio, int i) {
    return((sketchSize - i)/(i+1)*rRatio); // phi-gildið fyrir i-tu ítrun
}

double approximateP(double ax,
                    const int32_t threshold,
                    const int32_t sketchSize,
                    const double rRatio,
                    const int32_t sharedK)
{
    double partial = 0;

    const int32_t n = sharedK + threshold + 1;
    for (int32_t i = sharedK; i < n; i++) {
        ax *= sumRatio(sketchSize, rRatio, i);
        partial += ax;
    }

    return partial;
}

double computePValue(int sharedK, int sketchSize, double rValue){
    if(sharedK == 0) {
        return(1);
    }
    double rRatio = rValue/(1-rValue);
    double ax = initializeA(rValue, sketchSize, rRatio, sharedK);
    int threshold = std::max(3, getThreshold(rValue, sketchSize));
    double pValue = approximateP(ax, threshold, sketchSize, rRatio, sharedK);
    return(pValue);
}

double shared_kmers(const std::vector<uint64_t> &m1,
                    const std::vector<uint64_t> &m2,
                    const size_t size)
{

    size_t shared = 0;
    size_t unique = 0;
    size_t i = 0;
    size_t j = 0;

    while (unique < size && i != m1.size() && j != m2.size()) {
        if (m1[i] == m2[j]) {
            shared += 1;
            i += 1;
            j += 1;
        } else if (m1[i] < m2[j]) {
            i += 1;
        } else if (m1[i] > m2[j]) {
            j += 1;
        }

        unique += 1;
    }

    return shared;
}
