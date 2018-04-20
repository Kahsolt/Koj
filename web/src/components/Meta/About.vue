<template>
  <div id="about">
    <h3>Welcome Koj</h3>
    <pre>
      C：gcc 源文件名 -O2 -Wl,--stack=268435456 -DONLINE_JUDGE -DTSINSEN
      C++：g++ 源文件名 -O2 -Wl,--stack=268435456 -DONLINE_JUDGE -DTSINSEN
      C++0x：g++ 源文件名 -O2 --std=c++0x -Wl,--stack=268435456 -DONLINE_JUDGE -DTSINSEN
      VC: cl 源文件名 -O2 -DONLINE_JUDGE -DTSINSEN
      Pascal：fpc 源文件名 -O2 -dONLINE_JUDGE -dTSINSEN
      C#: csc /d:ONLINE_JUDGE /d:TSINSEN Code.cs
      Java：javac Main.java
    </pre>
  </div>
</template>

<script>
  export default {
    name: "about"
  }
</script>

<style scoped></style>
