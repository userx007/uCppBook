/*
g++ -pthread --std=c++20 multi_stage_pipeline_processing.cpp -o app
*/

#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <array>

struct DataBatch {
    std::vector<int> values;
    bool processed = false;
};

void pipeline_worker(int stage, int thread_id, 
                    std::array<std::barrier<>, 3>& stage_barriers,
                    std::vector<DataBatch>& batches) {
    const int num_batches = batches.size();
    const int chunk_size = num_batches / 4; // 4 threads per stage
    const int start = thread_id * chunk_size;
    const int end = (thread_id == 3) ? num_batches : start + chunk_size;
    
    std::cout << "Stage " << stage << ", Thread " << thread_id 
             << " processing batches " << start << "-" << end << "\n";
    
    // Stage 0: Data generation
    if (stage == 0) {
        for (int i = start; i < end; ++i) {
            batches[i].values.resize(100);
            for (auto& v : batches[i].values) {
                v = i * 100 + thread_id;
            }
        }
        stage_barriers[0].arrive_and_wait();
    }
    
    // Stage 1: Data transformation
    if (stage <= 1) {
        stage_barriers[0].arrive_and_wait(); // Wait for stage 0
        
        for (int i = start; i < end; ++i) {
            for (auto& v : batches[i].values) {
                v = v * 2 + 1;
            }
        }
        stage_barriers[1].arrive_and_wait();
    }
    
    // Stage 2: Data validation
    if (stage <= 2) {
        stage_barriers[1].arrive_and_wait(); // Wait for stage 1
        
        for (int i = start; i < end; ++i) {
            bool valid = true;
            for (const auto& v : batches[i].values) {
                if (v < 0) valid = false;
            }
            batches[i].processed = valid;
        }
        stage_barriers[2].arrive_and_wait();
    }
}

int main() {
    const int num_threads = 4;
    const int num_stages = 3;
    const int num_batches = 16;
    
    std::vector<DataBatch> batches(num_batches);
    std::array<std::barrier<>, 3> stage_barriers = {
        std::barrier<>(num_threads),
        std::barrier<>(num_threads),
        std::barrier<>(num_threads)
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(pipeline_worker, 0, i, 
                            std::ref(stage_barriers), std::ref(batches));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Pipeline processing complete\n";
    int processed_count = 0;
    for (const auto& batch : batches) {
        if (batch.processed) processed_count++;
    }
    std::cout << "Successfully processed: " << processed_count 
             << "/" << num_batches << " batches\n";
    
    return 0;
}