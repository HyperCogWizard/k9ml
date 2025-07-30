# Cognitive Process Scheduler API Documentation

## Overview

The Cognitive Process Scheduler extends K9's traditional priority-based scheduler with attention-based allocation mechanisms. It implements a tensor-based approach using the shape `[n_procs, n_features, t_time]` to track cognitive patterns and allocate system attention to processes based on their cognitive importance.

## Architecture

### Data Structures

#### CogTensor
```c
struct CogTensor {
    Lock;
    float data[CogMaxProcs][CogMaxFeatures][CogTimeWindow];
    ulong timestamp[CogTimeWindow];
    int current_time;
    int active_procs;
};
```
Primary attention tensor with dimensions:
- `CogMaxProcs` (256): Maximum processes tracked
- `CogMaxFeatures` (8): Cognitive features per process
- `CogTimeWindow` (32): Time window for temporal patterns

#### AttentionAlloc
```c
struct AttentionAlloc {
    Lock;
    CogSchedq queues[CogAttentionLevels];
    CogTensor tensor;
    ulong total_attention;
    ulong last_update;
    int emergency_mode;
};
```
Global attention allocator managing cognitive scheduling state.

#### Process Extensions
Each process gains cognitive fields:
```c
struct Proc {
    // ... existing fields ...
    int cog_index;
    float attention_level;
    float cognitive_features[CogMaxFeatures];
    ulong last_attention_update;
    Proc *cog_next;
    int is_cognitive;
};
```

### Cognitive Features

The scheduler tracks 8 cognitive features per process:

1. **CogFeatureLoad** (0.2 weight): CPU load pattern
2. **CogFeatureMemory** (0.15 weight): Memory access pattern  
3. **CogFeatureIO** (0.1 weight): I/O activity pattern
4. **CogFeatureInteractive** (0.25 weight): Interactive response pattern
5. **CogFeatureRealtime** (0.15 weight): Real-time requirement pattern
6. **CogFeatureNetwork** (0.05 weight): Network activity pattern
7. **CogFeaturePriority** (0.05 weight): Traditional priority influence
8. **CogFeatureEmergent** (0.05 weight): Emergent behavioral pattern

## API Functions

### Initialization

#### `void coginit(void)`
Initialize the cognitive scheduler. Called automatically during kernel initialization.

### Process Management

#### `void cogready(Proc *p)`
Add a cognitive process to the attention-based scheduling queues.
- Only affects processes with `p->is_cognitive = 1`
- Calculates attention level and assigns to appropriate queue
- Called automatically from `ready()` for cognitive processes

#### `Proc* cogrunproc(void)`
Select the next process from cognitive scheduler.
- Returns highest-attention process or `nil` if none available
- Integrates with traditional scheduler in `runproc()`

#### `void cogupdate(Proc *p)`
Update cognitive features for a process.
- Recalculates attention based on current system state
- Updates tensor with temporal patterns
- Called automatically during process state transitions

### Feature Management

#### `float cogattention(Proc *p)`
Get current attention level for a process.
- Returns value between 0.0 and 1.0
- Returns 0.0 for non-cognitive processes

#### `void cogsetfeature(Proc *p, int feature, float value)`
Manually set a cognitive feature value.
- `feature`: Feature index (0-7)
- `value`: Feature value (clamped to 0.0-1.0)
- Useful for specialized cognitive applications

#### `int iscognitive(Proc *p)`
Check if process uses cognitive scheduling.

### System Control

#### `void cogemergency(int enable)`
Enable/disable emergency mode.
- When enabled, disables cognitive scheduling
- Falls back to traditional priority scheduler
- Use during system stress or debugging

#### `void cogdump(void)`
Debug dump of cognitive scheduler state.
- Shows attention queues and process states
- Displays cognitive feature values
- Useful for debugging and monitoring

## Usage Examples

### Enable Cognitive Scheduling for a Process

```c
// In kernel code
Proc *p = newproc();
p->is_cognitive = 1;

// Set initial cognitive features
cogsetfeature(p, CogFeatureInteractive, 0.8);
cogsetfeature(p, CogFeatureRealtime, 0.6);

// Process will now use attention-based scheduling
ready(p);
```

### Monitor Attention Levels

```c
// Check process attention
float attention = cogattention(up);
if(attention > 0.5) {
    // High attention process
}

// Debug system state
cogdump();
```

### Emergency Fallback

```c
// Disable cognitive scheduling under stress
if(system_overload) {
    cogemergency(1);  // Fallback to traditional scheduler
}

// Re-enable when stable
cogemergency(0);
```

## Integration with Traditional Scheduler

The cognitive scheduler works alongside K9's existing priority-based scheduler:

1. **Parallel Operation**: Both schedulers operate simultaneously
2. **Cognitive Priority**: `cogrunproc()` is tried first in `runproc()`
3. **Fallback**: Traditional scheduler used when cognitive scheduler returns `nil`
4. **Feature Updates**: Cognitive features updated during normal process transitions
5. **Graceful Degradation**: System continues working if cognitive scheduler fails

## Performance Characteristics

### Memory Usage
- Tensor storage: ~256KB (reasonable for kernel use)
- Per-process overhead: ~40 bytes of cognitive fields
- Negligible impact on non-cognitive processes

### Computational Overhead
- Attention calculation: O(CogMaxFeatures) per process
- Queue operations: O(1) insertion/removal
- Tensor updates: O(1) per time window advancement

### Scalability
- Supports up to 256 cognitive processes simultaneously
- Degrades gracefully with increasing cognitive load
- Emergency mode provides safety valve

## Emergency and Error Handling

### Emergency Mode
- Automatically triggered by system stress
- Disables cognitive scheduling temporarily
- Logs emergency events for analysis

### Error Recovery
- Corrupted cognitive state detected and reset
- Failed attention calculations fall back to default values
- Lock contention resolved with exponential backoff

## Future Extensions

### Planned Features
1. **Dynamic Tensor Resizing**: Adapt tensor size to system load
2. **Multi-Machine Attention**: Distribute attention across cluster
3. **Learning Algorithms**: Self-tuning attention weights
4. **Hypergraph Integration**: Support for complex cognitive relationships

### Research Directions
1. **Neural Network Integration**: Direct neural network scheduling
2. **Quantum Attention**: Quantum-inspired attention mechanisms
3. **Biological Models**: Brain-inspired scheduling algorithms
4. **Emergent Behaviors**: Self-organizing process ecosystems

## See Also

- `port/proc.c`: Traditional process scheduler
- `port/portdat.h`: Process and scheduling data structures
- `ARCHITECTURE.md`: K9 system architecture overview
- Plan 9 scheduling documentation