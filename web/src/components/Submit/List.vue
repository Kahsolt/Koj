<template>
  <div id="submit-list">
    <h3>Submit List</h3>
    <table class="w3-table">
      <thead>
      <tr class="w3-blue">
        <th>Index</th>
        <th>Problem</th>
        <th>User</th>
        <th>Result</th>
        <th>Language</th>
        <th>Timestamp</th>
      </tr>
      </thead>
      <tbody>
      <tr class="w3-hoverable" v-for="(submit, index) in submits">
        <td>{{ index }}</td>
        <td>{{ submit.problem }}</td>
        <td>{{ submit.user }}</td>
        <td>{{ submit.result }}</td>
        <td><router-link :to="{name: 'submit_show', params: {id: submit.id}}">{{ submit.language }}</router-link></td>
        <td>{{ submit.timestamp }}</td>
      </tr>
      </tbody>
    </table>
  </div>
</template>

<script>
  export default {
    name: "submit-list",
    data() {
      return {
        submits: [],
      }
    },
    mounted: function () {
      this.$http.get('/submits/').then((res)=>{
        this.submits = res.data.results;
      }).catch((err)=>{
        console.log(err);
      });
    },
  }
</script>

<style scoped>

</style>
