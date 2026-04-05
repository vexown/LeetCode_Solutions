//LeetCode link: https://leetcode.com/problems/daily-temperatures/

int* dailyTemperatures(int* temperatures, int temperaturesSize, int* returnSize) 
{
    // Answer array: same size as input, default 0 (meaning "no warmer day ever came").
    *returnSize = temperaturesSize;
    int* answer = (int*)calloc(temperaturesSize, sizeof(int)); // calloc zero-inits for us

    // The stack holds INDICES of days that are still waiting for a warmer day.
    // Worst case (strictly decreasing temps), every day ends up on the stack,
    // so we size it to temperaturesSize.
    int* stack = (int*)malloc(temperaturesSize * sizeof(int));
    int top = -1; // -1 means "stack is empty". top is the index of the current top element.

    // Walk through each day left to right.
    for (int i = 0; i < temperaturesSize; ++i)
    {
        // Check if today's temperature resolves any waiting days.
        // "Resolves" = today is strictly warmer than the day sitting on top of the stack.
        //
        // We use `while`, not `if`, because today might resolve MULTIPLE waiting days at once.
        // Example: stack has days with temps [75, 71, 69] (top is 69), and today is 76.
        // 76 resolves 69, then 71, then 75 — all in one go.
        while (top >= 0 && temperatures[i] > temperatures[stack[top]])
        {
            int resolvedDay = stack[top]; // the day that was waiting
            top--;                         // pop it off the stack

            // How long did that day wait? From its index to today's index.
            answer[resolvedDay] = i - resolvedDay;
        }

        // Today is now a waiting day — push its index onto the stack.
        // Note: we push AFTER popping, so today never compares against itself.
        top++;
        stack[top] = i;
    }

    // Any indices still on the stack never found a warmer day.
    // Their answer stays 0 (thanks to calloc). No cleanup needed for them.

    free(stack);
    return answer;
}