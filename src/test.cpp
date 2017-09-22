#include <vector>
#include "gtest/gtest.h"
#include "audio.h"
#include "common.h"


TEST (WaveTestFixture,  AvgPackTest){
    BaseWave wav;
    wav.open("./data/M1F1-Alaw-AFsp.wav");
    const uint start_byte = 8000;
    float samples_avg_1 = wav.get_samples_avg(start_byte, 10);
    std::vector<float> samples_vec;
    wav.get_samples(samples_vec, start_byte, 10);
    float samples_avg_2 = 0;
    for (std::vector<float>::iterator it=samples_vec.begin();it != samples_vec.end() ; it++) {
        samples_avg_2 += abs(*it);
    }
    ASSERT_EQ(samples_avg_1, samples_avg_2/5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}