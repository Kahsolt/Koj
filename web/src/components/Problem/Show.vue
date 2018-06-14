<template>
  <div id="problem-show">
    <div class="w3-container">
      <h3>[{{ problem.sn }}] {{ problem.title }}</h3>
      <strong>Description:</strong><p>{{ problem.description }}</p>
      <strong>Input Description:</strong><p>{{ problem.input_desc }}</p>
      <strong>Output Description:</strong><p>{{ problem.output_desc }}</p>
      <strong>Input Sample:</strong><p>{{ problem.sample_input }}</p>
      <strong>Output Sample:</strong><p>{{ problem.sample_output }}</p>
      <strong>Hint:</strong><p>{{ problem.hint }}</p>
      <strong>Source:</strong><p>{{ problem.source }}</p>
    </div>
    <hr class="w3-tiny"/>
    <div class="w3-container">
      <button class="w3-btn w3-blue w3-show-inline-block" @click="submit">Submit</button>
      <select class="w3-input w3-border w3-border-blue w3-show-inline-block" v-model="solution.language" style="width: 150px">
        <option :value="lang.name" v-for="lang in languages">{{ lang.display_name }}</option>
      </select>
      <textarea class="w3-input w3-border w3-border-grey" rows="8" cols="20" wrap="soft"
                v-model="solution.code"></textarea>
    </div>
  </div>
</template>

<script>
  export default {
    name: "problem-show",
    data() {
      return {
        languages: [],
        problem: {
          sn: null, title: null, description: null,
          input_desc: null, output_desc: null,
          sample_input: null, sample_outpt: null,
          hint: null, source: null,
        },
        solution: {
          code: null, language: 'c',
        }
      }
    },
    methods: {
      submit() {
        if(this.$store.get('user')===null) {
          alert('Please login/register first.');
          return;
        }
        let submit = this.solution;
        submit.problem = this.problem.sn;
        this.$http.post('/submit/', submit).then((res)=>{
          console.log(res.data);
          this.result = JSON.stringify(res.data);
        }).catch((err)=>{
          console.log(err);
        });
      }
    },
    mounted() {
      let id = this.$route.params.id;
      this.$http.get('/problems/' + id + '/').then((res)=>{
        this.problem = res.data;
      }).catch((err)=>{
        console.log(err);
      });

      let languages = this.$store.get('languages');
      if(languages) {
        this.languages = languages;
      } else {
        this.$http.get('/languages/').then((res)=>{
          this.languages = res.data.languages;
          this.$store.set('languages', this.languages);
        }).catch((err)=>{
          console.log(err);
        });
      }
    }
  }
</script>

<style scoped></style>
