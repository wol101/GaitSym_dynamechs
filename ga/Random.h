/*
 *  Random.cpp
 *  PrimateAgent
 *
 *  Created by Bill Sellers on Mon Jan 31 2005.
 *  Copyright (c) 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef RANDOM_H
#define RANDOM_H

//#define USE_DRAND48
//#define USE_NUMERICAL_RECIPES
#define USE_MERSENNE_TWISTER


// RANDOM_SEED and RANDOM_FLOAT can be defined as system random
// number generators (eg. srand48 and drand48) or one of the 3
// inbuilt random number generators
// sran2 and ran2 are recommended for general use...
// The Mersenne Twister is a very high performance random number
// generator especially designed for large Monte Carlo Simulations
// RANDOM_SEED_TYPE needs to match the required type for the random seed

#ifdef USE_DRAND48
#define RANDOM_SEED_TYPE long
#define RANDOM_SEED srand48
#define RANDOM_FLOAT drand48
#endif
#ifdef USE_NUMERICAL_RECIPES
#define RANDOM_SEED_TYPE unsigned int
#define RANDOM_SEED sran2
#define RANDOM_FLOAT ran2
#endif
#ifdef USE_MERSENNE_TWISTER
#define RANDOM_SEED_TYPE unsigned long
#define RANDOM_SEED init_genrand
#define RANDOM_FLOAT genrand_res53
#endif

void RandomSeed(RANDOM_SEED_TYPE randomSeed);
double RandomDouble(double lowBound, double highBound);
int RandomInt(int lowBound, int highBound);
bool CoinFlip(double chanceOfReturningTrue = 0.5);
int SqrtBiasedRandomInt(int lowBound, int highBound);
double RandomUnitGaussian();
int RankBiasedRandomInt(int lowBound, int highBound);

#endif // RANDOM_H
