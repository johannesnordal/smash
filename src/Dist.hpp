#ifndef DIST_HPP
#define DIST_HPP
#include <algorithm>
#include <math.h>
#include <cstdint>
#include <vector>

int getThreshold(double rValue, int sketchSize);
double computeDist(double k, double Jaccard);
double randomOccurance(int k, int n);
double computeRValue(int k, int n1, int n2);
double initializeA(double rValue, int sketchSize, double rRatio, int sharedK);
double sumRatio(int sketchSize, double rRatio, int i);
double computePValue(int sharedK, int sketchSize, double rValue);

double shared_kmers(const std::vector<uint64_t> &m1,
                    const std::vector<uint64_t> &m2,
                    const size_t size);

double approximateP(double ax,
                    const int32_t threshold,
                    const int32_t sketchSize,
                    const double rRatio,
                    const int32_t sharedK);

#endif
