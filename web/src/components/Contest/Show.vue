<template>
  <div id="contest-show">
    <section>
      <h3>{{ contest.title }}</h3>
      <p><strong>Info:</strong>{{ contest.info }}</p>
      <p><strong>Time:</strong>{{ contest.start_time }} ~ {{ contest.end_time }} </p>
    </section>
    <hr/>
    <section>
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
        <tr class="w3-hoverable" v-for="(problem, index) in contest.problem_set">
          <td>{{ problem.sn }}</td>
          <td><router-link :to="{name: 'problem_show', params: {id: problem.id}}">{{ problem.title }}</router-link></td>
          <td>{{ calcACRatio(problem) }}%</td>
        </tr>
        </tbody>
      </table>
    </section>
  </div>
</template>

<script>
  export default {
    name: "contest-show",
    data() {
      return {
        contest: {
          title: null, info: null, start_time: null, end_time: null,
          problem_set: []
        },
      }
    },
    methods: {
      calcACRatio(problem) {
        return problem.count_ac === 0
          ? parseFloat(0).toFixed(2)
          : parseFloat(problem.count_submit/problem.count_ac).toFixed(2);
      }
    },
    mounted() {
      let id = this.$route.params.id;
      this.$http.get('/contests/' + id + '/').then((res)=>{
        this.contest = res.data;
      }).catch((err)=>{
        console.log(err);
      });
    }
  }
</script>

<style scoped></style>
