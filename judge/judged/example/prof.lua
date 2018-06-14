#!/usr/bin/env lua5.3

-- 11s
S=os.time()
for i = 1, 100 do
  os.execute("./main < 1.in > 1.inout")
  os.execute("diff 1.inout 1.out > /dev/null")
end
E=os.time()
print(E-S..'s')
