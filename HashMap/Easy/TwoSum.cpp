// LeetCode link: https://leetcode.com/problems/two-sum/
// Time complexity: O(n)
// Space complexity: O(n)

// Final thoughts: Good solution, but could be optimized by using only one pass through the nums vector
// and checking if the missing variable exists in the HashMap. Time complexity would still be O(n) but
// the code would be more faster due to less iterations.

#include <limits>

class Solution 
{
public:
    vector<int> twoSum(vector<int>& nums, int target) 
    {
        std::unordered_map<int, std::vector<int>> numsIndices; // define a HashMap for finding indicies that add up to target

        std::vector<int> ret_indices; // vector to store the indicies of nums that fulfill the condition
        
        for(size_t i = 0; i < nums.size(); i++) //for each num in nums
        {
            numsIndices[nums[i]].push_back(i); //store each num value as key and its index as value in HashMap for O(1) lookup
        }



        //Pass through the whole nums vector checking if there is a value in HashMap that would add up to target
        for(const auto& num : nums)
        {
            int x = target - num;

            if (numsIndices.find(x) != numsIndices.end()) // check if x exists, which is our missing variable: num + x = target
            {
                if (x == num) // edge case: x is the same value as num
                {
                    if(numsIndices[num].size() > 1) // check if there are two indicies for this value x (which is == num)
                    {
                        ret_indices.assign({numsIndices[num][0], numsIndices[num][1]}); 
                        return ret_indices;
                    }
                }

                if(numsIndices[x][0] != numsIndices[num][0]) // we cannot use the same element twice
                {
                    ret_indices.assign({numsIndices[num][0], numsIndices[x][0]}); 
                    return ret_indices;
                }
            }
        }

        return ret_indices; // would only happen if no pair is found
    }
};