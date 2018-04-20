<template>
  <div id="user-show" class="w3-panel">
    <table class="w3-table-all">
      <tbody>
      <tr>
        <td>Username:</td>
        <td>{{ user.username }}</td>
      </tr>
      <tr>
        <td>Email:</td>
        <td>{{ user.email }}</td>
      </tr>
      <tr>
        <td>Priority Point:</td>
        <td>{{ user.priority }}</td>
      </tr>
      <tr>
        <td>Total Submits:</td>
        <td>{{ user.count_submit }}</td>
      </tr>
      <tr>
        <td>Total AC:</td>
        <td>{{ user.count_ac }}</td>
      </tr>
      </tbody>
    </table>
  </div>
</template>

<script>
  export default {
    name: "submit-show",
    data() {
      return {
        user: {
          username:null, email: null,
          priority: 0, count_submit: 0, count_ac:0
        }
      }
    },
    mounted: function () {
      this.$http.get('/me/').then((res)=>{
        this.user = res.data;
      }).catch((err)=>{
        console.log(err);
      });
    }
  }
</script>

<style scoped></style>
