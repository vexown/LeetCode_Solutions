/**
 * LeetCode problem link: https://leetcode.com/problems/next-greater-element-i/ 
 */

// Step 1 - solve the "next greater element" for entire nums2 array using monotonic stack
// Step 2 - go through nums1 array and find solutions for each elements using results from step 1

// Example:
// Input: nums1 = [1,2,3], nums2 = [3,2,1,4]
// Output: ans = [4,4,4]

int* nextGreaterElement(int* nums1, int nums1Size, int* nums2, int nums2Size, int* returnSize) 
{
    // we need to find answer for each value in nums1 array so the ans size is equal to nums1Size
    int* ans_num1 = (int*)malloc(nums1Size * sizeof(int));
    *returnSize = nums1Size;

    int* stack = (int*)calloc(nums2Size, sizeof(int));
    int top = -1; // -1 means empty stack

    int* ans_num2 = (int*)calloc(nums2Size, sizeof(int));

    // Step 1
    for(int num2_idx = 0; num2_idx < nums2Size; ++num2_idx)
    {
        ans_num2[num2_idx] = -1;
        while( (top >= 0) && (nums2[num2_idx] > nums2[stack[top]]) )
        {
            int next_greater_val = nums2[num2_idx];
            ans_num2[stack[top]] = next_greater_val;
            top--;
        }
        top++;
        stack[top] = num2_idx;
    }

    // Step 2
    for(int num1_idx = 0; num1_idx < nums1Size; ++num1_idx)
    {
        for(int num2_idx = 0; num2_idx < nums2Size; ++num2_idx)
        {
            if(nums1[num1_idx] == nums2[num2_idx]) 
            {
                ans_num1[num1_idx] = ans_num2[num2_idx];
                break;
            }
        }
    }

    free(stack);
    free(ans_num2);
    return ans_num1;
}
