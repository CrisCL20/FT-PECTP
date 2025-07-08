import sys
import numpy as np 

values = [np.float64(line.strip()) for line in sys.stdin if line.strip()]

print(f"Total: {sum(values)}s")
print(f"Mean: {np.mean(values)}")