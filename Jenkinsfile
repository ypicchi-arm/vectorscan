pipeline {
  agent {
    node {
      label 'x86'
    }

  }
  stages {
    stage('Release, SSE') {
      agent {
        node {
          label 'x86'
        }

      }
      steps {
        sh 'mkdir build-release-SSE &&  cmake -DCMAKE_BUILD_TYPE=Release   -C build-release-SSE'
      }
    }

  }
}