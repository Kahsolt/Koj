import axios from 'axios'

export default{
  install(Vue, options) {
    axios.defaults.baseURL = 'http://localhost:8000/api/';
    axios.defaults.method = 'get';
    axios.defaults.timeout = 3000;
    axios.defaults.responseType = 'json';
    axios.defaults.headers.common['X-Requested-With'] = 'XMLHttpRequest';
    axios.defaults.headers.get['Content-Type'] = 'application/json';

    axios.setAuth = function(token) {
      axios.defaults.headers.common['Authorization'] = 'Token ' + token;
    };

    Vue.prototype.$http = axios;
  }
}
