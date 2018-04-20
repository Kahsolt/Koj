<template>
  <div id="navbar">
    <router-link :class="cssLink" :to="{name: 'about'}" class="w3-image">
      <img src="../../assets/logo.png" style="height: 20px">&nbsp;Koj</router-link>
    <router-link :class="cssLink" :to="{name: 'problem_list'}">Problems</router-link>
    <router-link :class="cssLink" :to="{name: 'contest_list'}">Contests</router-link>
    <router-link :class="cssLink" :to="{name: 'submit_list'}">Submits</router-link>
    <router-link :class="cssLink" :to="{name: 'rank'}">Rank</router-link>

    <div class="w3-dropdown-hover w3-right" v-if="isAuthed">
      <span :class="cssLink" style="width: 100px">{{ displayUsername }}</span>
      <div class="w3-dropdown-content w3-bar-block w3-card-4">
        <router-link :class="cssLink" :to="{name: 'profile'}">Profile</router-link>
        <a class="w3-bar-item w3-button w3-text-red" @click="deauth">Logout</a>
      </div>
    </div>
    <a :class="cssLink" class="w3-right" @click="showLoginModal=true" v-else>Login/Register</a>

    <div class="w3-modal" :style="{'display': showLoginModal ? 'block' : 'none' }">
      <div class="w3-modal-content w3-card-4" style="max-width: 500px">
        <div class="w3-container" :class="{'w3-blue': actLogin, 'w3-indigo': !actLogin}">
          <span @click="showLoginModal=false" class="w3-button w3-large w3-display-topright">&times;</span>
          <h3 class="w3-container w3-center">
            <span v-if="actLogin">Login</span>
            <span style="cursor: pointer; text-decoration: underline;" @click="actLogin=true" v-else>Login</span>
            <span>/</span>
            <span v-if="!actLogin">Register</span>
            <span style="cursor: pointer; text-decoration: underline;" @click="actLogin=false" v-else>Register</span>
          </h3>
        </div>
        <div class="w3-container w3-text-black w3-left-align" @keyup.enter="actLogin ? auth() : register()">
          <p>
            <label>Username</label>
            <input class="w3-input" type="text" v-model="user.username"/>
          </p>
          <p v-if="!actLogin">
            <label>Email</label>
            <input class="w3-input" type="email" v-model="user.email"/>
          </p>
          <p>
            <label>Password</label>&nbsp;
            <i class="w3-small" style="cursor: pointer" @click="showPassword=!showPassword">[show]</i>
            <input class="w3-input" type="text" v-model="user.password" v-if="showPassword"/>
            <input class="w3-input" type="password" v-model="user.password" v-else/>
          </p>
          <hr/>
          <p>
            <button class="w3-button w3-block w3-section w3-blue w3-ripple w3-padding"
                    v-if="actLogin" @click="auth">Login</button>
            <button class="w3-button w3-block w3-section w3-indigo w3-ripple w3-padding"
                    v-else @click="register">Register</button>
          </p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    name: "navbar",
    data() {
      return {
        cssLink: { "w3-bar-item w3-btn w3-hover-white w3-bottombar w3-hover-border-blue": true },

        actLogin: true, showLoginModal: false, showPassword: false, isAuthed: false,
        user: { username: null, password: null, email: null, token: null}
      }
    },
    computed: {
      displayUsername() {
        return this.user.username || 'Hacker';
      },
    },
    methods: {
      register() {
        this.$http.post('/users/', this.user).then((resp)=>{
          alert('Register OK, now try auto login.');
          this.auth();
        }).catch((err)=>{
          alert('Register failed.');
          console.log(err);
        });
      },
      auth() {
        this.$http.post('/users/auth/', this.user).then((resp)=>{
          this.$http.setAuth(resp.data.token);
          this.user.token = resp.data.token;
          this.$store.set('user', this.user);
          this.showLoginModal = false;
          this.isAuthed = true;
        }).catch((err)=>{
          alert('Login failed.');
          console.log(err);
        });
      },
      deauth() {
        if(confirm('Confirm logging out?')) {
          this.$http.setAuth('2333');
          this.$store.remove('user');
          this.isAuthed = false;
        }
      },
    },
    mounted() {  // Auto Login
      let user = this.$store.get('user');
      if(user) {
        this.user = user;
        this.auth();
        console.log('Auto Login');
      }
    }
  }
</script>

<style scoped></style>
