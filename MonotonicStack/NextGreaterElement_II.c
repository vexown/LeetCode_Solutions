// LeetCode problem: https://leetcode.com/problems/next-greater-element-ii/

// Algorithm:
// Find solutions for all elements in nums array as you pass through the array using a monotonic stack (waiting queue + resolving process)
// Since the array is circular, a full loop for element nums[x] goes all the way through the end of the array, the start, and reaches nums[x-1]
// This also means, passing through the whole array TWICE is enough to resolve all elements
// Trick: instead of manually resetting the loop counter when reaching the end, we loop from 0 to 2*numsSize
// and use (i % numsSize) to index into nums. This simulates a circular array using a regular one.
// Notes:
// - During the 2nd pass we only RESOLVE waiting elements, we don't push new ones onto the stack
//   (everything is already on the stack from the 1st pass, so there's nothing new to add)
// - This means we don't need any "don't overwrite" guard - by construction, the 2nd pass can only
//   fill in answers for indices that are still unresolved (still sitting on the stack)
int* nextGreaterElements(int* nums, int numsSize, int* returnSize) 
{
    int* ans = (int*)malloc(numsSize * sizeof(int));
    *returnSize = numsSize; // solution is provided for each element in nums so size is the same

    // Initialize all answers to -1 ("no greater element found"). Any index that never gets resolved
    // by the monotonic stack process will keep this default value.
    for(int i = 0; i < numsSize; ++i) ans[i] = -1;

    int* stack = (int*)calloc(numsSize, sizeof(int)); // stack to hold the waiting-to-be-resolved queue (we stack indexes of nums, not values)
    int top = -1;

    // Loop through nums TWICE using a single counter going from 0 to 2*numsSize.
    // The (i % numsSize) trick maps the counter back into a valid nums index, simulating circularity.
    for(int i = 0; i < 2 * numsSize; ++i)
    {
        int idx = i % numsSize; // actual index into nums (wraps around on the 2nd pass)

        // Check if the current element resolves any of the waiting elements on the stack
        while( (top >= 0) && (nums[idx] > nums[stack[top]]) )
        {
            int next_greater_num = nums[idx];
            ans[stack[top]] = next_greater_num; //save the next greater num solution for the given index to the ans array

            top--; //pop the resolved index off the stack
        }

        // Add the current index to the waiting list (push onto stack), but ONLY during the 1st pass.
        // On the 2nd pass we're purely resolving leftovers - no new elements should join the queue,
        // otherwise we'd be processing each element more than its "one full circle" worth of lookups.
        if(i < numsSize)
        {
            top++;
            stack[top] = idx; //we save the INDEX, not the value
        }
    }

    free(stack);
    return ans;
}
