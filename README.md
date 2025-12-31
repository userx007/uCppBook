
# **The Most Important Topics for a C++ Expert**

### **Core Language Mastery**

[1. **Value categories** (lvalues, rvalues, xvalues, glvalues, prvalues)](docs/Topics/1_Value_Categories.md)<br>
[2. **Object lifetime & storage duration**](docs/Topics/2_Object_lifetimes.md)<br>
[3. **Copy elision & return value optimization (RVO)**](docs/Topics/3_Copy_elision.md)<br>
[4. **Move semantics** & **perfect forwarding**](docs/Topics/4_Move_Semantics_Perfect_Forwarding.md)<br>
[5. **RAII** and deterministic destruction](docs/Topics/5_RAII_and_Deterministic_Destruction.md)<br>
[6. **Operator overloading** (best practices & pitfalls)](docs/Topics/6_Operator_overloading_best_practices_and_pitfalls)<br>
[7. **Templates** (function, class, variable templates)](docs/Topics/7_templates)<br>
[8. **Template specialization & partial specialization**](docs/Topics/8_Template_Specialization)<br>
[9. **SFINAE**, tag dispatch, enable_if patterns](docs/Topics/_template_metaprogramming_patterns)<br>
[10. **Concepts & requires-clauses (C++20)**](docs/Topics/10_Understanding_C++20_concepts_and_requires_clauses)<br>
[11. **Constexpr functions/variables** (C++11-20 evolution)](docs/Topics/11_Constexpr_functions_variables_C++11_20_evolution.md)<br>
[12. **Inline functions, ODR & linkage**](docs/Topics/12_Inline_functions_ODR_and_linkage_in_C++.md)<br>
[13. **Memory alignment & padding**](docs/Topics/13_Memory_alignment_and_padding_in_C++.md)<br>
[14. **Type deduction** (`auto`, `decltype`, CTAD)](docs/Topics/14_C++_type_deduction_with_auto_and_decltype.md)<br>
[15. **Understanding the standard object model**](docs/Topics/15_Understanding_the_C++_standard_object_model.md)<br>


### **STL & Standard Library**

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


### **Memory & Performance**

[26. **Stack vs heap vs static storage**](docs/Topics/26_Stack_vs_heap_vs_static_storage.md)<br>
[27. **Low-level memory manipulation** (`memcpy`, placement new)](docs/Topics/27_Low_level_memory_manipulation_memcpy_placement_new.md)<br>
[28. **Memory ownership models**](docs/Topics/28_Memory_ownership_models.md)<br>
[29. **Cache-aware programming**](docs/Topics/29_Cache_aware_programming.md)<br>
[30. **Inlining & compiler optimization behavior**](docs/Topics/30_Inlining.md)<br>
[31. **Move vs copy performance characteristics**](docs/Topics/31_Move_vs_copy_performance_characteristics.md)<br>
[32. **Avoiding undefined behavior** and understanding UB traps](docs/Topics/32_Avoiding_undefined_behavior_and_understanding_UB_traps.md)<br>
[33. **Small object optimization, EBO (Empty Base Optimization)**](docs/Topics/33_Small_object_optimization_EBO_Empty_Base_Optimization.md)<br>
[34. **Custom memory pools / arenas**](docs/Topics/34_Custom_memory_pools_arenas.md)<br>


### **Concurrency & Parallelism**

[35. **Threads & thread lifecycle**](docs/Topics/35_Threads_and_thread_lifecycle.md)<br>
[36. **Mutexes, locks, and lock-free programming**](docs/Topics/36_Mutexes_locks_and_lock_free_programming)<br>
[37. **Atomics & memory ordering**](docs/Topics/37_Atomics_and_memory_ordering.md)<br>
[38. **Condition variables & synchronization primitives**](docs/Topics/38_Condition_variables_and_synchronization_primitives.md)<br>
[39. **Futures, promises, async**](docs/Topics/39_Futures_promises_async)<br>
[40. **C++20 Coroutines**](docs/Topics/40_C++20_Coroutines.md)<br>
[41. **Parallel STL**](docs/Topics/41_Parallel_STL.md)<br>


### **Advanced Language Features**

[42. **CRTP (Curiously Recurring Template Pattern)**](docs/Topics/42_CRTP_Curiously_Recurring_Template_Pattern.md)<br>
[43. **Expression templates**](docs/Topics/43_Expression_templates.md)<br>
[44. **Type traits & meta-programming**](docs/Topics/44_Type_traits_and_meta_programming.md)<br>
[45. **Compile-time programming (TMP & C++20 constexpr + consteval)**](docs/Topics/45_Compile_time_programming_TMP_and_C++20_constexpr_consteval.md)<br>
[46. **Policy-based design & mixins**](docs/Topics/46_Policy_based_design_and_mixins.md)<br>


### **Build Systems & Tooling**

[47. **Compiler flags (GCC/Clang/MSVC)** and ABI considerations](docs/Topics/)<br>
[48. **CMake** and modern C++ project organization](docs/Topics/)<br>
[49. **Static analysis & sanitizers** (ASan, UBSan, TSan)](docs/Topics/)<br>
[50. **Debugging tools** (gdb, lldb, valgrind, perf)](docs/Topics/)<br>


### **Miscelaneous**

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






