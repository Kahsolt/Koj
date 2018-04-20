import store from 'store'

export default{
  install(Vue, options) {
    Vue.prototype.$store = store;
  }
}
