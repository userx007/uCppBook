use std::thread;
use std::time::Duration;

// Simple worker function
fn worker(id: usize, message: &str) {
    thread::sleep(Duration::from_millis(100));
    println!("Thread {}: {}", id, message);
}

// Worker function that returns a value
fn calculate_sum(start: i32, end: i32) -> i32 {
    (start..=end).sum()
}

fn main() {
    println!("=== Basic Pattern with JoinHandle ===");
    
    // Pattern 1: Simple thread creation and joining
    {
        let handles: Vec<_> = (0..5)
            .map(|i| {
                thread::spawn(move || {
                    worker(i, "Hello from thread");
                })
            })
            .collect();
        
        // Join all threads
        for handle in handles {
            handle.join().unwrap();
        }
    }
    
    println!("\n=== Pattern with Return Values ===");
    
    // Pattern 2: Getting return values (built into JoinHandle)
    {
        let handles: Vec<_> = (0..3)
            .map(|i| {
                thread::spawn(move || {
                    calculate_sum(i * 10, (i + 1) * 10)
                })
            })
            .collect();
        
        // Collect results
        let results: Vec<i32> = handles
            .into_iter()
            .map(|h| h.join().unwrap())
            .collect();
        
        for (i, sum) in results.iter().enumerate() {
            println!("Sum {}: {}", i, sum);
        }
    }
    
    println!("\n=== Pattern with Move Closure ===");
    
    // Pattern 3: Using move to capture variables
    {
        let data = vec![10, 20, 30, 40, 50];
        
        let handles: Vec<_> = data
            .into_iter()
            .enumerate()
            .map(|(i, val)| {
                thread::spawn(move || {
                    println!("Processing index {} with value {}", i, val * 2);
                })
            })
            .collect();
        
        for handle in handles {
            handle.join().unwrap();
        }
    }
    
    println!("\n=== Scoped Threads (Borrowing Data) ===");
    
    // Pattern 4: Scoped threads for borrowing without moving
    {
        let data = vec![1, 2, 3, 4, 5];
        
        thread::scope(|s| {
            for (i, item) in data.iter().enumerate() {
                s.spawn(move || {
                    println!("Thread {} processing: {}", i, item * 2);
                });
            }
            // All threads automatically joined at scope end
        });
        
        // data is still available here!
        println!("Original data still accessible: {:?}", data);
    }
    
    println!("\n=== Shared State with Mutex ===");
    
    // Pattern 5: Sharing mutable state safely
    {
        use std::sync::{Arc, Mutex};
        
        let counter = Arc::new(Mutex::new(0));
        let handles: Vec<_> = (0..5)
            .map(|_| {
                let counter = Arc::clone(&counter);
                thread::spawn(move || {
                    let mut num = counter.lock().unwrap();
                    *num += 1;
                })
            })
            .collect();
        
        for handle in handles {
            handle.join().unwrap();
        }
        
        println!("Final counter value: {}", *counter.lock().unwrap());
    }
}