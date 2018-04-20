<template>
  <div id="user-list">
    <h3>Rank List</h3>
    <table class="w3-table">
      <thead>
      <tr class="w3-blue">
        <th>Nickname</th>
        <th>Signature</th>
        <th>AC Count</th>
        <th>Submit Count</th>
        <th>AC Ratio</th>
      </tr>
      </thead>
      <tbody>
      <tr class="w3-hoverable" v-for="(user, index) in users">
        <td>{{ user.nickname }}</td>
        <td>{{ user.signature }}</td>
        <td>{{ user.count_submit }}</td>
        <td>{{ user.count_ac }}</td>
        <td>{{ parseFloat(user.count_submit/user.count_ac).toFixed(2) }}%</td>
      </tr>
      </tbody>
    </table>
    <hr/>
    <div>
      <span>Total Pages: {{ pageCount }}</span>
      <button class="w3-button" :style="{'disabled': !pagePrevious}" @click="pageChange(-1)">Previous Page</button>
      <button class="w3-button" :style="{'disabled': !pageNext}" @click="pageChange(+1)">Next Page</button>
    </div>
  </div>
</template>

<script>
  export default {
    name: "user-list",
    data() {
      return {
        users: [],
        countSize: 0, pageSize: 20, pagePrevious: null, pageNext: null,
      }
    },
    computed: {
      pageCount() {
        return this.countSize / this.pageSize;
      }
    },
    methods: {
      pageChange(offset) {
        if(offset === 1) {
          this.$http.get(this.pageNext).then((res)=>{
            this.countSize = res.data.count;
            this.pagePrevious = res.data.previous;
            this.pageNext = res.data.next;
            this.users = res.data.results;
          }).catch((err)=>{
            console.log(err);
          });
        } else if (offset === -1) {
          this.$http.get(this.pagePrevious).then((res)=>{
            this.countSize = res.data.count;
            this.pagePrevious = res.data.previous;
            this.pageNext = res.data.next;
            this.users = res.data.results;
          }).catch((err)=>{
            console.log(err);
          });
        }
      }
    },
    mounted: function () {
      this.$http.get('/users/').then((res)=>{
        this.countSize = res.data.count;
        this.pagePrevious = res.data.previous;
        this.pageNext = res.data.next;
        this.users = res.data.results;
      }).catch((err)=>{
        console.log(err);
      });
    }
  }
</script>

<style scoped></style>
