<template>
  <div id="problem-list">
    <h3>Problem List</h3>
    <table class="w3-table">
      <thead>
      <tr class="w3-blue">
        <th>ID</th>
        <th>Title</th>
        <th>AC Ratio</th>
      </tr>
      </thead>
      <tbody>
      <tr class="w3-hoverable" v-for="(problem, index) in problems">
        <td>{{ problem.sn }}</td>
        <td><router-link :to="{name: 'problem_show', params: {id: problem.id}}">{{ problem.title }}</router-link></td>
        <td>{{ parseFloat(problem.count_submit/problem.count_ac).toFixed(2) }}%</td>
      </tr>
      </tbody>
    </table>
  </div>
</template>

<script>
  export default {
    name: "problem-list",
    data() {
      return {
        problems: [],
      }
    },
    mounted: function () {
      this.$http.get('/problems/').then((res)=>{
        this.problems = res.data.results;
      }).catch((err)=>{
        console.log(err);
      });
    },
  }
</script>

<style scoped></style>
