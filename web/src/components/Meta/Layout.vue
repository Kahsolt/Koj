<template>
  <div id="layout" class="w3-content">
    <navbar class="w3-bar w3-blue"/>
    <router-view/>
    <hr class="w3-small"/>
    <footbar class="w3-container w3-right"/>
  </div>
</template>

<script>
  import Navbar from "./Navbar";
  import Footbar from "./Footbar";

  export default {
    name: "layout",
    components: {Navbar, Footbar}
  }
</script>

<style scoped></style>
