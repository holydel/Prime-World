function exec-sql ($sql) {
$sw = [System.Diagnostics.Stopwatch]::StartNew()
  Write-Host "$(get-date) executing query`n$sql`n"
  # коннект к серверу (подразумевает наличие pgpass.conf)
  $result = psql -h $dbhost -p $dbport -d $dbname -U $dbuser -c "SET client_min_messages TO WARNING; COPY ($sql) TO STDOUT CSV;"
  if (!$?) { 
    Write-Host 'psql failed'
    return
  }
$sw.stop()
write-host "$(get-date) script executed in $($sw.elapsed)`n"
return $result
}


function process-player-actions ($acts, $p) {
  $d = ''
  $step = 0
  $acts |% {
	if ($step -gt $maxsteps) { return }
  	if ($_.action -eq 'login') {
	  if (++$step -gt $maxsteps) { return }
	  $d += "{0};{1};{2};{3};{4};`n" -f $step, $_.action, $_.result, $_.player, $_.epoch
	}
	else {
	  if ($prev_act -ne 'login') {
		if (++$step -gt $maxsteps) { return }
		$d += "{0};{1};{2};{3};{4};`n" -f $step, 'login', '', $_.player, ($_.epoch-1)
	  }
	  $d += "{0};{1};{2};{3};{4};`n" -f $step, $_.action, $_.result, $_.player, $_.epoch
	}
	$prev_act = $_.action
  }
  $d = $d.TrimEnd("`n")
  if ($step -le $maxsteps) {
    $d += "stop"
  }
  return $d
}


# ---------------------------------------------------------------------------------------
# start ---------------------------------------------------------------------------------
# ---------------------------------------------------------------------------------------

# соединение с БД
$dbhost = 'b489.SITE'
$dbport = 5432
$dbname = 'pw_analysis_oa'
$dbuser = 'postgres'

# прочие параметры
$maxsteps = 7
$tablename = 'temp.pf89663_actionstream_0208_neweek'


$data = exec-sql "
select extract(epoch from timestamp) ts,action,result,playerid,id 
from $tablename order by playerid,ts"

$actions = $data |% { 
  $d = $_.split(',')
  New-Object psobject -Property @{ 
    epoch = [int]$d[0]
    action = $d[1]
    result = $d[2]
    player = $d[3]
    id = $d[4]
  }
}

$null > "$tablename.csv"

$tmp_acts = @()
$p = ''
$actions |% {
  if ($p -eq $_.player -or $p -eq '') {
  	$tmp_acts += $_
	if ($p -eq '') { $p = $_.player }
	return
  }
  process-player-actions $tmp_acts $p >> "$tablename.csv"
  $tmp_acts = @()
  $tmp_acts += $_
  $p = $_.player
}
process-player-actions $tmp_acts $p >> "$tablename.csv"


$csv = Import-Csv -Path "$tablename.csv" -Delimiter ';' -Header @('step','action','result','player','timestamp','stop')

1..7 |% {
  $step = $_
  $tmpcsv = $csv |? { $_.step -eq $step }
  $v1 = ($tmpcsv |? { $_.action -eq 'login' }).count 
  $v2 = ($tmpcsv |? { $_.action -eq 'login' -and $_.stop -eq 'stop' }).count 
  if ($v2 -eq $null) { $v2 = 0 }
  Write-Host "$v1`t$v2`n`nwon		lose		failed		leave		ileave"
  
  $data = ''
  'pvp','training','tutorial','dragonwald','native_earth',`
  'outpost','shuffle','zombieland','pve single','pve coop' |% {
    $act = $_
    'won','lose','failed','leave','ileave' |% {
      $res = $_
      $v1 = ($tmpcsv |? { $_.action -eq $act -and $_.result -eq $res } | measure).count 
      $v2 = ($tmpcsv |? { $_.action -eq $act -and $_.result -eq $res -and $_.stop -eq 'stop' } | measure).count 
      $data += "$v1`t$v2`t"
    }
    $data += "`n"
  }
  Write-Host $data
}



