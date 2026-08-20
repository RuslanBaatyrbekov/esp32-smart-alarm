<?php
date_default_timezone_set('Europe/Berlin');
header('Content-Type: application/json; charset=utf-8');

$evaNo = 'your_station_id';
$date = date('ymd');

$targetTimeStr = isset($_GET['time']) ? $_GET['time'] : date('H:i');
$targetTimestamp = strtotime($targetTimeStr);

$startHour = (int)date('H', $targetTimestamp);
$hoursToFetch = [$startHour, ($startHour + 1) % 24];

$clientId = 'DB_client_id';
$clientSecret = 'DB_client_secret';

$headers = [
    'DB-Client-Id: ' . $clientId,
    'DB-Api-Key: ' . $clientSecret,
    'accept: application/xml'
];

$trains = [];

// 1. Получаем план для нужных часов (текущий + следующий)
foreach ($hoursToFetch as $h) {
    $hourStr = str_pad($h, 2, '0', STR_PAD_LEFT);
    $ch = curl_init("https://apis.deutschebahn.com/db-api-marketplace/apis/timetables/v1/plan/{$evaNo}/{$date}/{$hourStr}");
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
    $responsePlan = curl_exec($ch);
    curl_close($ch);

    $xmlPlan = simplexml_load_string($responsePlan);
    if ($xmlPlan && isset($xmlPlan->s)) {
        foreach ($xmlPlan->s as $train) {
            $id = (string)$train['id'];
            if ($train->dp) {
                $pt = (string)$train->dp['pt'];
                $line = (string)$train->dp['l'];
                $path = isset($train->dp['ppth']) ? (string)$train->dp['ppth'] : '';

                if (str_contains($path, 'Frankfurt')) {
                    $timeFormatted = substr($pt, 6, 2) . ':' . substr($pt, 8, 2);

                    $trains[$id] = [
                        'line' => $line,
                        'planned_time' => $timeFormatted,
                        'actual_time' => $timeFormatted,
                        'delay_mins' => 0,
                        'cancelled' => false,
                        'raw_pt' => $pt
                    ];
                }
            }
        }
    }
}

// 2. Получаем live-изменения (опоздания / отмены)
$ch = curl_init("https://apis.deutschebahn.com/db-api-marketplace/apis/timetables/v1/fchg/{$evaNo}");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
$responseFchg = curl_exec($ch);
curl_close($ch);

$xmlFchg = simplexml_load_string($responseFchg);
if ($xmlFchg && isset($xmlFchg->s)) {
    foreach ($xmlFchg->s as $change) {
        $id = (string)$change['id'];
        if (isset($trains[$id]) && isset($change->dp)) {
            if (isset($change->dp['cs']) && (string)$change->dp['cs'] === 'c') {
                $trains[$id]['cancelled'] = true;
            }
            if (isset($change->dp['ct'])) {
                $ct = (string)$change->dp['ct'];
                $trains[$id]['actual_time'] = substr($ct, 6, 2) . ':' . substr($ct, 8, 2);

                $tsPlan = strtotime("20" . substr($trains[$id]['raw_pt'], 0, 10));
                $tsFact = strtotime("20" . substr($ct, 0, 10));
                $trains[$id]['delay_mins'] = round(($tsFact - $tsPlan) / 60);
            }
        }
    }
}

uasort($trains, function($a, $b) {
    return strcmp($a['planned_time'], $b['planned_time']);
});

$filteredTrains = [];
foreach ($trains as $t) {
    if (strcmp($t['planned_time'], date('H:i', $targetTimestamp)) >= 0) {
        $filteredTrains[] = $t;
    }
}

if (count($filteredTrains) < 3) {
    $activeTrains = array_values($trains);
} else {
    $activeTrains = $filteredTrains;
}

$resultList = [];
foreach ($activeTrains as $t) {
    $line = $t['line'];
    $actual = $t['actual_time'];
    $plan = $t['planned_time'];

    if ($t['cancelled']) {
        $text = "{$line} - отменен";
    } elseif ($t['delay_mins'] > 0) {
        $text = "{$line} - {$actual} (план {$plan})";
    } else {
        $text = "{$line} - {$actual}";
    }

    $resultList[] = $text;
}

echo json_encode(['trains' => $resultList], JSON_UNESCAPED_UNICODE);