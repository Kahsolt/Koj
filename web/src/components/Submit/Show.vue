<template>
  <div id="submit-show">
    <div class="w3-panel">
      <i class="w3-tag w3-green">{{ submit.result }}</i>
      Problem: {{ submit.problem }} by {{ submit.user }} using {{ submit.language }} at {{ submit.timestamp }}
    </div>
    <hr/>
    <p>Code: </p>
    <div class="w3-panel">{{ submit.code }}</div>
  </div>
</template>

<script>
  export default {
    name: "submit-show",
    data() {
      return {
        submit: {
          problem: null, user: null, language: null,
          code: null, result: null, timestamp: null,
        }
      }
    },
    mounted: function () {
      let id = this.$route.params.id;
      this.$http.get('/submits/' + id + '/').then((res)=>{
        this.submit = res.data;
      }).catch((err)=>{
        console.log(err);
      });
    }
  }
</script>

<style scoped></style>
