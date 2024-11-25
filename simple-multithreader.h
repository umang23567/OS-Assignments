#include <iostream>                
#include <vector>                   
#include <pthread.h>                // to include POSIX library 
#include <functional>               // for lambdas
#include <ctime>                    // to measure execution times
#include <cassert>                  // to make assertions
#include <iomanip>
#include <cmath>

using namespace std;

// Helper function for 1D parallel loop
void* parallel_for_1D_helper(void* arg) 
{
    // Unpack the argument to retrieve the lambda function and range
    auto* data = static_cast<pair<function<void(int)>, pair<int, int>>*>(arg);
    auto& lambda = data->first;       // Lambda function to execute
    auto& range = data->second;       // Start and end of the range

    // Loop over the assigned range and execute the lambda
    for (int i=range.first; i<range.second; i++) 
    {
        lambda(i);
    }

    delete data;            // Clean up allocated memory
    return nullptr;
}

// Helper function for 2D parallel loops
void* parallel_for_2D_helper(void* arg) 
{
    // Unpack the argument to retrieve the lambda function and ranges
    auto* data = static_cast<pair<function<void(int, int)>, pair<pair<int, int>, pair<int, int>>>*>(arg);
    auto& lambda = data->first;       // Lambda function to execute
    auto& ranges = data->second;      // Row and column ranges

    // Loop over the assigned 2D ranges and execute the lambda
    for (int i=ranges.first.first; i<ranges.first.second; i++) 
    {
        for (int j=ranges.second.first; j<ranges.second.second; j++) 
        {
            lambda(i,j);
        }
    }

    delete data;           // Clean up allocated memory
    return nullptr;
}

// Parallel for loop implementation for 1D range
void parallel_for(int l, int r, function<void(int)> &&lambda, int numThreads) 
{
    clock_t start = clock();                        // Record the start time 
    vector<pthread_t> threads(numThreads);          // Vector to store thread IDs

    int range = r-l;                                    // Total range  
    int chunk_size = ceil((double)range/numThreads);       // Divide range into chunks

    for (int i=0; i<numThreads; i++) 
    {
        int start_index = l + i*chunk_size;                         // Start of chunk
        int end_index = min(start_index+chunk_size, r);          // End of chunk

        if (start_index>=end_index) continue;      // Skip if invalid range

        // Allocate and pack the lambda and range into the data structure
        auto* data = new pair<function<void(int)>,pair<int,int>>(lambda,make_pair(start_index,end_index));
        
        // Create a new thread to handle this range
        pthread_create(&threads[i], nullptr, parallel_for_1D_helper, (void*)data);
    }

    // Wait for all threads 
    for (int i=0; i<numThreads; i++) 
    {
        pthread_join(threads[i],nullptr);
    }

    clock_t end = clock();                          // Record the end time

    // Calculate duration
    double duration = static_cast<double>(end-start) / CLOCKS_PER_SEC;    
    cout << "Execution Time (1D loop): " << fixed << setprecision(4) << duration*1000 << " ms\n";  
}

// Parallel for loop implementation for 2D ranges
void parallel_for(int l1, int r1, int l2, int r2, function<void(int,int)> &&lambda, int numThreads) 
{
    clock_t start = clock();                                        // Record the start time for measuring execution time
    vector<pthread_t> threads(numThreads*numThreads);               // Vector to store thread IDs

    int range1 = r1-l1;                              // Total range for first dimension
    int range2 = r2-l2;                              // Total range for second dimension

    // Divide ranges into chunks for each dimension
    int chunk_size1 = ceil((double)range1/numThreads);
    int chunk_size2 = ceil((double)range2/numThreads);

    int thread_count = 0;                               //  Track of actual threads created

    for (int i=0; i <numThreads; i++) 
    {
        int start_index1 = l1 + i*chunk_size1;                              // Start of chunk for first dimension
        int end_index1 = min(start_index1 + chunk_size1, r1);               // End of chunk for first dimension

        if (start_index1>=end_index1) continue;       // Skip if invalid range

        for (int j=0; j<numThreads; j++) 
        {
            int start_index2 = l2 + j*chunk_size2;                        // Start of chunk for second dimension
            int end_index2 = min(start_index2+chunk_size2,r2);            // End of chunk for second dimension

            if (start_index2>=end_index2) continue;     // Skip if invalid range

            // Allocate and pack the lambda and ranges into the data structure
            auto* data = new pair<function<void(int, int)>, pair<pair<int, int>, pair<int, int>>>
            (
                lambda, make_pair(make_pair(start_index1, end_index1), make_pair(start_index2, end_index2))
            );

            // Create a new thread to handle this range
            pthread_create(&threads[thread_count++], nullptr, parallel_for_2D_helper, (void*)data);
        }
    }

    // Wait for all threads 
    for (int i=0; i<thread_count; i++)
    {
        pthread_join(threads[i],nullptr);
    }

    clock_t end = clock();                          // Record the end time

    // Calculate duration
    double duration = static_cast<double>(end-start) / CLOCKS_PER_SEC; 
    cout << "Execution Time (2D loop): " << fixed << setprecision(4) << duration*1000 << " ms\n";   
}

// Entry point for user-specific code
int user_main(int argc, char **argv);

// Helper function to execute a lambda function
void demonstration(function<void()> && lambda) 
{
    lambda();       // Execute the lambda function
}

int main(int argc, char **argv) 
{
    // Sample variables for demonstration
    int x=5, y=1;

    // Lambda that captures variables and modifies y
    auto lambda1 = [x,&y]() 
    {
        y = 5;
        cout << "====== Welcome to Assignment-" << y << " of the CSE231(A) ======\n";
    };

    // Execute the first lambda
    demonstration(move(lambda1)); 

    // Call the user-defined main function
    int rc = user_main(argc,argv);

    // Another example lambda to display a farewell message
    auto lambda2 = []() 
    {
        cout << "====== Hope you enjoyed CSE231(A) ======\n";
    };

    // Execute the second lambda
    demonstration(move(lambda2));
    
    return rc;          // Return result from user_main
}

// Redirect main to user_main
#define main user_main
