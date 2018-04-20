<template>
  <div id="contest-list">
    <h3>Contest List</h3>
    <table class="w3-table">
      <thead>
      <tr class="w3-blue">
        <th>ID</th>
        <th>Title</th>
        <th>Open Time</th>
      </tr>
      </thead>
      <tbody>
      <tr class="w3-hoverable" v-for="(contest, index) in contests">
        <td>{{ contest.sn }}</td>
        <td><router-link :to="{name: 'contest_show', params: {id: contest.id}}">{{ contest.title }}</router-link></td>
        <td>{{ contest.start_time }} ~ {{ contest.end_time }}</td>
      </tr>
      </tbody>
    </table>
  </div>
</template>

<script>
  export default {
    name: "contest-list",
    data() {
      return {
        contests: [],
      }
    },
    mounted() {
      this.$http.get('/contests/').then((res)=>{
        this.contests = res.data.results;
      }).catch((err)=>{
        console.log(err);
      });
    }
  }
</script>

<style scoped></style>
