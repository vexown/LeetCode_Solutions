// LeetCode problem: https://leetcode.com/problems/next-greater-element-i/
//
// ============================================================================
// LEARNING GUIDE: Building a hash map from scratch in C
// ============================================================================
//
// A hash map (a.k.a. hash table, dictionary, associative array) stores
// key -> value pairs and lets you look up a value by its key in O(1) average
// time. The core idea is:
//
//   1. Take the key and run it through a HASH FUNCTION to get an integer.
//   2. Reduce that integer to an index into a fixed-size array (the "bucket"
//      or "slot" the entry lives in).
//   3. Store the entry at that index.
//
// The catch: two different keys can hash to the same slot ("collision"). The
// two main strategies for handling collisions are:
//
//   - SEPARATE CHAINING: each slot holds a linked list of entries that hashed
//     there. Simple but cache-unfriendly (lots of pointer chasing).
//
//   - OPEN ADDRESSING: every entry lives directly in the array. On collision,
//     you probe other slots in the array until you find an empty one. This is
//     what we use here, with the simplest probing strategy: LINEAR PROBING
//     (just walk forward one slot at a time).
//
// Open addressing with linear probing is what we'll build below. It's compact,
// cache-friendly, and easy to get right — as long as you keep the load factor
// (entries / capacity) low enough that probe chains stay short. We achieve
// that by sizing the table to ~2x the expected number of entries.
// ============================================================================

#include <stdbool.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------
// Data structures
// ----------------------------------------------------------------------------

// A single slot in the table. Each slot can be empty or hold one (key, value)
// pair. We need the `occupied` flag because we can't reserve a special "empty"
// integer key — any int could be a legitimate key from the caller.
typedef struct
{
    int  key;
    int  value;
    bool occupied;  // false = this slot is empty
} HashEntry;

// The hash map itself is just a pointer to an array of slots plus its size.
// We require `capacity` to be a power of 2 so we can replace the slow `% capacity`
// with a fast bitwise `& (capacity - 1)`. (For any power of 2 N, x % N == x & (N-1).)
typedef struct
{
    HashEntry* entries;
    int        capacity;
} HashMap;

// ----------------------------------------------------------------------------
// Hash function
// ----------------------------------------------------------------------------
//
// The job of a hash function is to take a key and scramble it into an integer
// that looks "random" — so that even keys that are very similar (1, 2, 3, ...)
// land in very different slots. If the hash function is bad, similar keys
// cluster together, probe chains get long, and lookups slow down.
//
// We use Knuth's multiplicative hash. It's a one-liner: multiply the key by a
// carefully chosen constant and take the high bits. Cheap and good enough for
// non-adversarial integer keys (i.e. when an attacker isn't deliberately
// crafting keys to collide — real-world hash maps facing untrusted input use
// stronger hashes like SipHash to resist "hash flooding" attacks).
static int hashInt(int key, int capacity)
{
    // The magic number 2654435769 comes from the golden ratio:
    // it's floor((sqrt(5) - 1) / 2 * 2^32), i.e. the fractional part of phi
    // scaled to 32 bits. Knuth showed that multiplying by a constant derived
    // from an irrational number spreads sequential keys evenly across the
    // output range, and phi is provably the "most irrational" number — its
    // continued fraction expansion is [1; 1, 1, 1, ...] — which minimizes
    // clustering more than any other irrational.
    //
    // We cast to unsigned so the multiplication wraps cleanly modulo 2^32
    // instead of being undefined behavior on signed overflow.
    unsigned int h = (unsigned int)key * 2654435769u;

    // Now we have a 32-bit "random-looking" number, but we need an index in
    // [0, capacity). Since capacity is a power of 2, masking with (capacity-1)
    // keeps just the low log2(capacity) bits. Equivalent to `h % capacity`,
    // but a single AND instruction instead of a division.
    return (int)(h & (capacity - 1));
}

// ----------------------------------------------------------------------------
// Create / destroy
// ----------------------------------------------------------------------------

static HashMap* hashMapCreate(int minCapacity)
{
    // We want capacity to be:
    //   (a) a power of 2 (so the & trick works), and
    //   (b) at least 2x the number of entries we expect to store.
    //
    // The 2x is the LOAD FACTOR target: we'll never let the table get more
    // than ~50% full. Lower load factor = shorter probe chains = faster
    // lookups, at the cost of more memory. 50% is a common sweet spot for
    // linear probing; chaining-based maps tolerate higher load factors.
    int capacity = 16;
    while (capacity < minCapacity * 2) capacity *= 2;

    HashMap* map  = (HashMap*)malloc(sizeof(HashMap));
    map->capacity = capacity;

    // calloc zero-initializes the memory, which means every entry starts with
    // occupied = false (since false is 0). If we used malloc instead, we'd
    // have to loop over the array and set occupied = false ourselves.
    map->entries = (HashEntry*)calloc(capacity, sizeof(HashEntry));

    return map;
}

static void hashMapFree(HashMap* map)
{
    free(map->entries);
    free(map);
}

// ----------------------------------------------------------------------------
// Insert / update
// ----------------------------------------------------------------------------
//
// To insert (key, value):
//   1. Hash the key to get a starting slot index.
//   2. If that slot is empty, drop the entry there. Done.
//   3. If it's occupied by the SAME key, overwrite the value (update). Done.
//   4. If it's occupied by a DIFFERENT key (a collision), walk forward one
//      slot at a time until we find an empty slot or the same key. This is
//      "linear probing".
//
// Because we keep load factor below 50%, the average probe chain stays very
// short — typically 1-2 steps.
static void hashMapPut(HashMap* map, int key, int value)
{
    int idx = hashInt(key, map->capacity);

    // Walk forward until we find a usable slot.
    // - If occupied is false: free slot, we'll insert here.
    // - If occupied is true and key matches: same key, we'll overwrite.
    // - Otherwise: collision, keep probing.
    while (map->entries[idx].occupied && map->entries[idx].key != key)
    {
        // (idx + 1) & (capacity - 1) is the wrap-around equivalent of
        // (idx + 1) % capacity. When we walk off the end of the array, we
        // wrap back to the start.
        idx = (idx + 1) & (map->capacity - 1);
    }

    // At this point idx is either an empty slot or a slot already holding
    // this key. Either way, writing the entry does the right thing.
    map->entries[idx].key      = key;
    map->entries[idx].value    = value;
    map->entries[idx].occupied = true;
}

// ----------------------------------------------------------------------------
// Lookup
// ----------------------------------------------------------------------------
//
// Lookup mirrors insert: hash to a starting slot, then walk forward.
//   - If we find the key, return its value.
//   - If we hit an empty slot, the key isn't in the map (because if it had
//     been inserted, it would have landed in or before this empty slot).
//
// That second point is the crucial invariant of linear probing: an empty slot
// terminates the probe chain. This is why we can't just clear an entry by
// flipping `occupied` back to false on deletion — doing so would break lookups
// for any key whose probe chain passed through that slot. (Real hash maps
// solve this with "tombstones" or by rehashing the rest of the chain. We
// don't implement deletion here, so we don't need to worry about it.)
static int hashMapGet(HashMap* map, int key, int defaultValue)
{
    int idx = hashInt(key, map->capacity);

    while (map->entries[idx].occupied)
    {
        if (map->entries[idx].key == key) return map->entries[idx].value;
        idx = (idx + 1) & (map->capacity - 1);
    }

    // Hit an empty slot without finding the key — it's not in the map.
    return defaultValue;
}

// ============================================================================
// The actual LeetCode problem solution
// ============================================================================
//
// Problem: for each element of nums1, find the next greater element to its
// right in nums2 (or -1 if none exists). nums1 is a subset of nums2.
//
// Strategy: a classic monotonic-stack pass over nums2 computes the "next
// greater" for EVERY element of nums2 in O(n). We then need to look those
// answers up by value (not by index) when we walk nums1 — which is exactly
// what a hash map is for. So we map: value -> its next greater element.
// ============================================================================

int* nextGreaterElement(int* nums1, int nums1Size, int* nums2, int nums2Size, int* returnSize)
{
    *returnSize = nums1Size;
    int* ans    = (int*)malloc(nums1Size * sizeof(int));

    // Map of nums2 value -> its next greater element. Sized for nums2Size
    // entries (the create function will round up and double for load factor).
    HashMap* map = hashMapCreate(nums2Size);

    // Standard monotonic-decreasing stack of indices into nums2.
    int* stack = (int*)malloc(nums2Size * sizeof(int));
    int  top   = -1;  // -1 means empty

    // ---- Phase 1: monotonic stack pass over nums2 ----
    //
    // Invariant: the stack holds indices of nums2 elements whose "next greater"
    // we haven't found yet, in DECREASING order of value from bottom to top.
    // When we see a new value larger than the top of the stack, we've found
    // the answer for everything on the stack smaller than it — pop them and
    // record (poppedValue -> currentValue) in the map.
    for (int i = 0; i < nums2Size; ++i)
    {
        while (top >= 0 && nums2[i] > nums2[stack[top]])
        {
            int resolvedValue = nums2[stack[top]];
            hashMapPut(map, resolvedValue, nums2[i]);
            top--;
        }
        // Push current index. Anything still on the stack is now <= nums2[i],
        // so the decreasing-order invariant holds.
        top++;
        stack[top] = i;
    }
    // Anything left on the stack at the end has no greater element to its
    // right, so it never gets inserted. hashMapGet will return -1 (the default
    // we pass in below) for those values, which is exactly what the problem
    // asks for.

    // ---- Phase 2: answer each nums1 query with an O(1) hash lookup ----
    for (int i = 0; i < nums1Size; ++i)
    {
        ans[i] = hashMapGet(map, nums1[i], -1);
    }

    // Total time: O(nums2Size) for phase 1 + O(nums1Size) for phase 2
    //           = O(nums1Size + nums2Size).

    free(stack);
    hashMapFree(map);
    return ans;
}
