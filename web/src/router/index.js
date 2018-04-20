import Vue from 'vue'
import Router from 'vue-router'

Vue.use(Router);

export default new Router({
  routes: [
    { path: '/', component: resolve => require(['@/components/Meta/Layout'], resolve),
      children: [
        { path: 'problems', name: 'problem_list', alias: '',
          component: resolve => require(['@/components/Problem/List'], resolve) },
        { path: 'problems/:id', name: 'problem_show',
          component: resolve => require(['@/components/Problem/Show'], resolve) },
        { path: 'problems/:id/submits', name: 'problem_submits',
          component: resolve => require(['@/components/Submit/List'], resolve) },
        { path: 'contests', name: 'contest_list',
          component: resolve => require(['@/components/Contest/List'], resolve) },
        { path: 'contests/:id', name: 'contest_show',
          component: resolve => require(['@/components/Contest/Show'], resolve) },
        { path: 'submits', name: 'submit_list',
          component: resolve => require(['@/components/Submit/List'], resolve) },
        { path: 'submits/:id', name: 'submit_show',
          component: resolve => require(['@/components/Submit/Show'], resolve) },
        { path: 'rank', name: 'rank',
          component: resolve => require(['@/components/User/List'], resolve) },
        { path: 'profile', name: 'profile',
          component: resolve => require(['@/components/User/Show'], resolve) },
        { path: 'about', name: 'about',
          component: resolve => require(['@/components/Meta/About'], resolve) },
      ]
    },
  ]
});
