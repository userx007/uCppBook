
# **C++ Essential Topics**

## **Core Language Mastery**

[1. **Value categories** (lvalues, rvalues, xvalues, glvalues, prvalues)](docs/Topics/01_Value_Categories.md)<br>
[2. **Object lifetime & storage duration**](docs/Topics/02_Object_lifetimes.md)<br>
[3. **Copy elision & return value optimization (RVO)**](docs/Topics/03_Copy_elision.md)<br>
[4. **Move semantics** & **perfect forwarding**](docs/Topics/04_Move_Semantics_Perfect_Forwarding.md)<br>
[5. **RAII** and deterministic destruction](docs/Topics/05_RAII_and_Deterministic_Destruction.md)<br>
[6. **Operator overloading** (best practices & pitfalls)](docs/Topics/06_Operator_overloading_best_practices_and_pitfalls)<br>
[7. **Templates** (function, class, variable templates)](docs/Topics/07_Templates)<br>
[8. **Template specialization & partial specialization**](docs/Topics/08_Template_Specialization)<br>
[9. **SFINAE**, tag dispatch, enable_if patterns](docs/Topics/09_C++_template_metaprogramming_patterns.md)<br>
[10. **Concepts & requires-clauses (C++20)**](docs/Topics/10_Understanding_C++20_concepts_and_requires_clauses.md)<br>
[11. **Constexpr functions/variables** (C++11-20 evolution)](docs/Topics/11_Constexpr_functions_variables_C++11_20_evolution.md)<br>
[12. **Inline functions, ODR & linkage**](docs/Topics/12_Inline_functions_ODR_and_linkage_in_C++.md)<br>
[13. **Memory alignment & padding**](docs/Topics/13_Memory_alignment_and_padding_in_C++.md)<br>
[14. **Type deduction** (`auto`, `decltype`, CTAD)](docs/Topics/14_C++_type_deduction_with_auto_and_decltype.md)<br>
[15. **Understanding the standard object model**](docs/Topics/15_Understanding_the_C++_standard_object_model.md)<br>


## **STL & Standard Library**

[16. **Containers** (vectors, maps, lists, unordered maps, variants, etc.)](docs/Topics/16_C++_containers_and_data_structures.md)<br>
[17. **Iterators & iterator categories**](docs/Topics/17_Iterators_and_iterator_categories.md)<br>
[18. **Allocator model & custom allocators**](docs/Topics/18_Allocator_model_and_custom_allocators.md)<br>
[19. **Algorithms library** and execution policies (C++17)](docs/Topics/19_Algorithms_library_and_execution_policies_C++17.md)<br>
[20. **Smart pointers** (`unique_ptr`, `shared_ptr`, `weak_ptr`)](docs/Topics/20_Smart_pointers_unique_ptr_shared_ptr_weak_ptr.md)<br>
[21. **Function objects, lambdas & closures**](docs/Topics/21_Function_objects_lambdas_and_closures.md)<br>
[22. **`std::string` & string_view**](docs/Topics/22_string_and_string_view.md)<br>
[23. **`std::optional`, `std::variant`, `std::any`**](docs/Topics/23_std_optional_variant_any.md)<br>
[24. **Ranges (C++20)** — views, adaptors, pipelines](docs/Topics/24_Ranges_C++20_views_adaptors_pipelines.md)<br>
[25. **Chrono library** and time utilities](docs/Topics/25_Chrono_library_and_time_utilities.md)<br>


## **Memory & Performance**

[26. **Stack vs heap vs static storage**](docs/Topics/26_Stack_vs_heap_vs_static_storage.md)<br>
[27. **Low-level memory manipulation** (`memcpy`, placement new)](docs/Topics/27_Low_level_memory_manipulation_memcpy_placement_new.md)<br>
[28. **Memory ownership models**](docs/Topics/28_Memory_ownership_models.md)<br>
[29. **Cache-aware programming**](docs/Topics/29_Cache_aware_programming.md)<br>
[30. **Inlining & compiler optimization behavior**](docs/Topics/30_Inlining.md)<br>
[31. **Move vs copy performance characteristics**](docs/Topics/31_Move_vs_copy_performance_characteristics.md)<br>
[32. **Avoiding undefined behavior** and understanding UB traps](docs/Topics/32_Avoiding_undefined_behavior_and_understanding_UB_traps.md)<br>
[33. **Small object optimization, EBO (Empty Base Optimization)**](docs/Topics/33_Small_object_optimization_EBO_Empty_Base_Optimization.md)<br>
[34. **Custom memory pools / arenas**](docs/Topics/34_Custom_memory_pools_arenas.md)<br>


## **Concurrency & Parallelism**

[35. **Threads & thread lifecycle**](docs/Topics/35_Threads_and_thread_lifecycle.md)<br>
[36. **Mutexes, locks, and lock-free programming**](docs/Topics/36_Mutexes_locks_and_lock_free_programming.md)<br>
[37. **Atomics & memory ordering**](docs/Topics/37_Atomics_and_memory_ordering.md)<br>
[38. **Condition variables & synchronization primitives**](docs/Topics/38_Condition_variables_and_synchronization_primitives.md)<br>
[39. **Futures, promises, async**](docs/Topics/39_Futures_promises_async.md)<br>
[40. **C++20 Coroutines**](docs/Topics/40_C++20_Coroutines.md)<br>
[41. **Parallel STL**](docs/Topics/41_Parallel_STL.md)<br>


## **Advanced Language Features**

[42. **CRTP (Curiously Recurring Template Pattern)**](docs/Topics/42_CRTP_Curiously_Recurring_Template_Pattern.md)<br>
[43. **Expression templates**](docs/Topics/43_Expression_templates.md)<br>
[44. **Type traits & meta-programming**](docs/Topics/44_Type_traits_and_meta_programming.md)<br>
[45. **Compile-time programming (TMP & C++20 constexpr + consteval)**](docs/Topics/45_Compile_time_programming_TMP_and_C++20_constexpr_consteval.md)<br>
[46. **Policy-based design & mixins**](docs/Topics/46_Policy_based_design_and_mixins.md)<br>


## **Build Systems & Tooling**

[47. **Compiler flags (GCC/Clang/MSVC)** and ABI considerations](docs/Topics/)<br>
[48. **CMake** and modern C++ project organization](docs/Topics/)<br>
[49. **Static analysis & sanitizers** (ASan, UBSan, TSan)](docs/Topics/)<br>
[50. **Debugging tools** (gdb, lldb, valgrind, perf)](docs/Topics/)<br>


## **Miscelaneous**

[47. **Type Casting**](docs/Topics/47_Type_Casting.md)<br>
[48. **Modules in C++20**](docs/Topics/48_Modules_in_C++20.md)<br>
[49. **Structured bindings in C++17**](docs/Topics/49_Structured_bindings_in_C++17.md)<br>
[50. **st::span C++20 non owning views of contiguous sequences**](docs/Topics/50_std_span_C++20_non_owning_views_of_contiguous_sequences.md)<br>
[51. **Three way comparison spaceship operator in C++20**](docs/Topics/51_Three_way_comparison_spaceship_operator_in_c++20.md)<br>
[52. **Designated initializers C++20**](docs/Topics/52_Designated_initializers_C++20.md)<br>
[53. **Virtual functions vtables and dynamic dispatch**](docs/Topics/53_Virtual_functions_vtables_and_dynamic_dispatch.md)<br>
[54. **Multiple inheritance virtual inheritance and diamond problem**](docs/Topics/54_Multiple_inheritance_virtual_inheritance_and_diamond_problem.md)<br>
[55. **Pimpl idiom**](docs/Topics/55_Pimpl_idiom.md)<br>
[56. **Abstract classes_and_interface_design**](docs/Topics/56_Abstract_classes_and_interface_design.md)<br>
[57. **Exception safety guarantees basic strong nothrow**](docs/Topics/57_Exception_safety_guarantees_basic_strong_nothrow.md)<br>
[58. **Initialization uniform aggregate list direct copy**](docs/Topics/58_Initialization_uniform_aggregate_list_direct_copy.md)<br>
[59. **ADL Argument Dependent Lookup and name lookup rules**](docs/Topics/59_ADL_Argument_Dependent_Lookup_and_name_lookup_rules.md)<br>
[60. **Static initialization order fiasco**](docs/Topics/60_Static_initialization_order_fiasco.md)<br>
[61. **Attributes nodiscard likely and_others**](docs/Topics/61_Attributes_nodiscard_likely_and_others.md)<br>
[62. **Static polymorphism in C++**](docs/Topics/62_Static_Polymorphism_in_C++.md)<br>
[63. **Curiously Recurring Template Pattern (CRTP)**](docs/Topics/63_CRTP.md)<br>
[64. **Expressions statements declarations**](docs/Topics/64_Expressions_statements_declarations.md)<br>
[65. **Reference binding rules**](docs/Topics/65_Reference_binding_rules.md)<br>
[66. **Exclusive List Expression Cathegories**](docs/Topics/66_Exclusive_List_Expression_Cathegories.md)<br>
[67. **Function arguments**](docs/Topics/67_Function_arguments.md)<br>
[68. **Template Details**](docs/Topics/68_Template_Details.md)<br>
[69. **Template Overloading vs. Template Specialization**](docs/Topics/69_Template_Overloading_vs_Template_Specialization.md)<br>
[70. **Casting operators**](docs/Topics/70_Casting_operators.md)<br>
[71. **Feature list C++20 (main topics)**](docs/Topics/71_C++20_new_features.md)<br>
[72. **Feature list C++23 (main topics)**](docs/Topics/72_C++23_new_features.md)<br>
[73. **C++ View Types Comparison**](docs/Topics/73_C++_View_Types_Comparison.md)<br>
[74. **C++ Lambdas**](docs/Topics/74_Lambdas.md)<br>
[75. **C++ std::function**](docs/Topics/75_std_function.md)<br>
[76. **C++ std::transform**](docs/Topics/76_std_transform.md)<br>
[77. **Universal References and Reference Collapsing**](docs/Topics/77_Universal_References_and_Reference_Collapsing.md)<br>
[78. **Solutions for Avoiding Unwanted Copies with `auto`**](docs/Topics/78_Solutions_for_Avoiding_Unwanted_Copies_with_auto.md)<br>
[79. **The `std::vector<bool>` Proxy Problem**](docs/Topics/79_The_std_vector_bool_Proxy_Problem.md)<br>
[80. **Class Template Argument Deduction (CTAD) and Deduction Guides**](docs/Topics/80_Class_Template_Argument_Deduction_CTAD_and_Deduction_Guides.md)<br>
[81. **Perfect Forwarding with `decltype(auto)`**](docs/Topics/81_Perfect_Forwarding_with_decltype_auto.md)<br>

---

# C++ Concurrency Essential Topics


## Fundamentals

[100. **Thread Basics and Lifecycle**](docs/Concurrency/100_Thread_Basics_And_Lifecycle.md)<br>
[101. **Thread Management and RAII**](docs/Concurrency/101_Thread_Management_And_RAII.md)<br>
[102. **Passing Arguments to Threads**](docs/Concurrency/102_Passing_Arguments_To_Threads.md)<br>
[103. **Thread Identification and Hardware**](docs/Concurrency/103_Thread_Identification_And_Hardware.md)<br>


## Synchronization Primitives

[104. **Mutex Fundamentals**](docs/Concurrency/104_Mutex_Fundamentals.md)<br>
[105. **Lock Guards and Unique Locks**](docs/Concurrency/105_Lock_Guards_And_Unique_Locks.md)<br>
[106. **Shared Mutex and Reader-Writer Locks**](docs/Concurrency/106_Shared_Mutex_And_Reader_Writer_Locks.md)<br>
[107. **Recursive Mutex**](docs/Concurrency/107_Recursive_Mutex.md)<br>
[108. **Timed Mutex Operations**](docs/Concurrency/108_Timed_Mutex_Operations.md)<br>
[109. **Scoped Lock for Multiple Mutexes**](docs/Concurrency/109_Scoped_Lock_For_Multiple_Mutexes.md)<br>


## Condition Variables and Synchronization

[110. **Condition Variable Basics**](docs/Concurrency/110_Condition_Variable_Basics.md)<br>
[111. **Spurious Wakeups and Predicates**](docs/Concurrency/111_Spurious_Wakeups_And_Predicates.md)<br>
[112. **Producer-Consumer Pattern**](docs/Concurrency/112_Producer_Consumer_Pattern.md)<br>
[113. **Condition Variable Any**](docs/Concurrency/113_Condition_Variable_Any.md)<br>


## Atomic Operations

[114. **Atomic Types Fundamentals**](docs/Concurrency/114_Atomic_Types_Fundamentals.md)<br>
[115. **Memory Ordering and Sequential Consistency**](docs/Concurrency/115_Memory_Ordering_And_Sequential_Consistency.md)<br>
[116. **Relaxed Memory Ordering**](docs/Concurrency/116_Relaxed_Memory_Ordering.md)<br>
[117. **Acquire-Release Semantics**](docs/Concurrency/117_Acquire_Release_Semantics.md)<br>
[118. **Consume Ordering**](docs/Concurrency/118_Consume_Ordering.md)<br>
[119. **Compare and Swap Operations**](docs/Concurrency/119_Compare_And_Swap_Operations.md)<br>
[120. **Atomic Flags and Spinlocks**](docs/Concurrency/120_Atomic_Flags_And_Spinlocks.md)<br>
[121. **Atomic Smart Pointers**](docs/Concurrency/121_Atomic_Smart_Pointers.md)<br>


## Futures and Promises

[122. **Future and Promise Basics**](docs/Concurrency/122_Future_And_Promise_Basics.md)<br>
[123. **Async Task Launching**](docs/Concurrency/123_Async_Task_Launching.md)<br>
[124. **Packaged Tasks**](docs/Concurrency/124_Packaged_Tasks.md)<br>
[125. **Shared Futures**](docs/Concurrency/125_Shared_Futures.md)<br>
[126. **Exception Handling Across Threads**](docs/Concurrency/126_Exception_Handling_Across_Threads.md)<br>


## Thread-Safe Data Structures

[127. **Thread-Safe Queue Implementation**](docs/Concurrency/127_Thread_Safe_Queue_Implementation.md)<br>
[128. **Lock-Free Stack**](docs/Concurrency/128_Lock_Free_Stack.md)<br>
[129. **Concurrent Hash Map**](docs/Concurrency/129_Concurrent_Hash_Map.md)<br>
[130. **Ring Buffer for Lock-Free Communication**](docs/Concurrency/130_Ring_Buffer_For_Lock_Free_Communication.md)<br>


## Advanced Patterns

[131. **Thread Pools**](docs/Concurrency/131_Thread_Pools.md)<br>
[132. **Work Stealing Schedulers**](docs/Concurrency/132_Work_Stealing_Schedulers.md)<br>
[133. **Read-Copy-Update (RCU) Pattern**](docs/Concurrency/133_Read_Copy_Update_Pattern.md)<br>
[134. **Double-Checked Locking**](docs/Concurrency/134_Double_Checked_Locking.md)<br>
[135. **Sequential Locks (SeqLock)**](docs/Concurrency/135_Sequential_Locks.md)<br>


## C++20 Features

[136. **Latches**](docs/Concurrency/136_Latches.md)<br>
[137. **Barriers**](docs/Concurrency/137_Barriers.md)<br>
[138. **Counting Semaphores**](docs/Concurrency/138_Counting_Semaphores.md)<br>
[139. **Binary Semaphores**](docs/Concurrency/139_Binary_Semaphores.md)<br>
[140. **Jthread and Stop Tokens**](docs/Concurrency/140_Jthread_And_Stop_Tokens.md)<br>
[141. **Atomic Wait and Notify**](docs/Concurrency/141_Atomic_Wait_And_Notify.md)<br>


## Memory Models and Optimization

[142. **C++ Memory Model**](docs/Concurrency/142_Cpp_Memory_Model.md)<br>
[143. **False Sharing and Cache Line Alignment**](docs/Concurrency/143_False_Sharing_And_Cache_Line_Alignment.md)<br>
[144. **ABA Problem**](docs/Concurrency/144_ABA_Problem.md)<br>
[145. **Hazard Pointers**](docs/Concurrency/145_Hazard_Pointers.md)<br>


## Debugging and Best Practices

[146. **Deadlock Detection and Prevention**](docs/Concurrency/146_Deadlock_Detection_And_Prevention.md)<br>
[147. **Thread Sanitizer and Race Detection**](docs/Concurrency/147_Thread_Sanitizer_And_Race_Detection.md)<br>
[148. **Performance Profiling of Concurrent Code**](docs/Concurrency/148_Performance_Profiling_Of_Concurrent_Code.md)<br>
[149. **Testing Concurrent Code**](docs/Concurrency/149_Testing_Concurrent_Code.md)<br>
[150. **First Tier Most Used Features and Methods**](docs/Concurrency/150_Most_Used_Features_and_Methods.md)<br>
[151. **Second Tier Most Used Features and Methods**](docs/Concurrency/151_Second_Tier_Most_Used_Features_and_Methods.md)<br>
[152. **Third Tier Most Used Features and Methods**](docs/Concurrency/152_Third_Tier_Most_Used_Features_and_Methods.md)<br>

## Miscelaneous

[153. **std::mutex: lock() vs try_lock()**](docs/Concurrency/153_std_mutex_lock_vs_try_lock.md)<br>


# C++ vs Rust

[200. **Smart Pointers: C++ vs Rust Cheat Sheet**](docs/Rust/200_Smart_Pointers_C++_vs_Rust_Cheat_Sheet.md)<br>
[201. **Weak Pointers: C++ vs Rust**](docs/Rust/201_Weak_Pointers_in_C++_and_Rust.md)<br>

