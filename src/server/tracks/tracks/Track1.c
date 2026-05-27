//
// Created by Wiktor on 27.05.2026.
//

#include "Track1.h"

#include "../../entities/Track.h"

void Track1_loadTrack() {
    const Vector2f control_points1[] = {
    {-77.7f, -140.6f},
    {-77.7f, 319.3f},
    {-75.4f, 692.8f}
};
track_generate(control_points1, 3, 310.0f);

const Vector2f control_points2[] = {
    {-85.0f, 987.6f},
    {-76.7f, 1194.4f},
    {-10.6f, 1467.3f},
    {369.9f, 1628.6f},
    {692.5f, 1463.2f},
    {787.6f, 1037.2f},
    {783.5f, 297.0f},
    {1179.8f, -6.9f},
    {1532.1f, 499.5f},
    {1576.1f, 1176.6f},
    {1427.5f, 2249.9f},
    {375.1f, 2369.5f},
    {-482.1f, 2244.9f},
    {-1156.1f, 2640.5f},
    {-870.4f, 3241.3f},
    {389.7f, 3299.9f},
    {1327.5f, 3256.0f},
    {1964.9f, 3058.1f},
    {2983.2f, 1036.1f},
    {2946.6f, -399.8f},
    {2975.9f, -898.0f},
    {3203.0f, -1125.1f},
    {3488.7f, -1205.7f},
    {3708.5f, -1029.9f},
    {3818.4f, -722.2f},
    {3840.4f, -297.3f}
};
track_generate(control_points2, 26, 260.0f);

const Vector2f control_points3[] = {
    {229.3f, 689.9f},
    {167.3f, 966.9f}
};
track_generate_barrier(control_points3, 2);

const Vector2f control_points4[] = {
    {-378.6f, 714.7f},
    {-345.5f, 958.7f}
};
track_generate_barrier(control_points4, 2);

const Vector2f control_points5[] = {
    {3855.0f, -128.8f},
    {3855.0f, 208.2f},
    {3877.0f, 515.9f},
    {3899.0f, 838.3f},
    {3950.3f, 1175.3f},
    {3964.9f, 1453.7f},
    {3803.8f, 1805.4f},
    {3474.1f, 2164.3f},
    {3386.2f, 2538.0f},
    {3503.4f, 2904.3f},
    {3855.0f, 3087.5f},
    {4338.6f, 3197.3f},
    {4778.2f, 3234.0f},
    {5335.0f, 3285.3f},
    {5723.2f, 3190.0f},
    {5811.2f, 2794.4f},
    {5562.1f, 2523.3f},
    {5232.4f, 2435.4f},
    {4748.9f, 2369.5f},
    {4463.1f, 2332.8f},
    {4309.3f, 1981.2f},
    {4455.8f, 1695.5f},
    {4741.5f, 1578.2f},
    {4983.3f, 1563.6f},
    {5151.8f, 1563.6f},
    {5459.5f, 1497.7f},
    {5701.3f, 1190.0f},
    {5796.5f, 809.0f},
    {5723.2f, 369.4f},
    {5518.1f, -216.7f},
    {5144.5f, -575.7f},
    {4814.8f, -971.3f},
    {4595.0f, -1279.0f},
    {4331.3f, -1645.3f},
    {4023.6f, -1865.1f},
    {3649.9f, -2018.9f},
    {3232.3f, -2084.9f},
    {2836.7f, -2084.9f},
    {2602.3f, -1967.7f},
    {2404.4f, -1601.4f},
    {2309.2f, -1337.6f},
    {2243.3f, -942.0f},
    {2206.6f, -766.2f},
    {2096.7f, -502.4f},
    {1723.1f, -553.7f},
    {1474.0f, -773.5f},
    {1320.2f, -1117.8f},
    {1078.4f, -1498.8f},
    {558.2f, -1755.2f},
    {-474.8f, -1747.9f},
    {-892.4f, -942.0f}
};
track_generate(control_points5, 51, 200.0f);

const Vector2f control_points6[] = {
    {-993.5f, -789.6f},
    {-1066.0f, -585.2f},
    {-1099.0f, -420.4f},
    {-1099.0f, -262.1f},
    {-1303.4f, -150.0f},
    {-1474.8f, -84.1f}
};
track_generate(control_points6, 6, 150.0f);

const Vector2f control_points7[] = {
    {-1679.2f, 8.2f},
    {-1877.0f, 87.4f},
    {-2008.9f, 311.5f},
    {-2055.0f, 700.6f},
    {-2127.6f, 1155.5f},
    {-2121.0f, 1472.0f},
    {-2088.0f, 1808.3f},
    {-1725.4f, 1933.6f},
    {-1389.1f, 1887.4f},
    {-1171.5f, 1722.6f},
    {-1079.2f, 1287.4f},
    {-1052.8f, 852.2f},
    {-1039.6f, 463.2f}
};
track_generate(control_points7, 13, 190.0f);

const Vector2f control_points8[] = {
    {3582.5f, -268.7f},
    {3655.0f, -123.6f}
};
track_generate_barrier(control_points8, 2);

const Vector2f control_points9[] = {
    {4103.4f, -321.5f},
    {4044.1f, -143.4f}
};
track_generate_barrier(control_points9, 2);

const Vector2f control_points10[] = {
    {-1072.6f, -1027.0f},
    {-1125.3f, -835.8f}
};
track_generate_barrier(control_points10, 2);

const Vector2f control_points11[] = {
    {-716.5f, -868.7f},
    {-848.4f, -743.4f}
};
track_generate_barrier(control_points11, 2);

const Vector2f control_points12[] = {
    {-1534.1f, -229.1f},
    {-1758.3f, -183.0f}
};
track_generate_barrier(control_points12, 2);

const Vector2f control_points13[] = {
    {-1415.5f, 61.0f},
    {-1580.3f, 166.5f}
};
track_generate_barrier(control_points13, 2);

const Vector2f control_points14[] = {
    {-387.9f, -142.2f},
    {-352.9f, -320.9f},
    {-198.7f, -415.5f},
    {-48.0f, -457.6f},
    {88.7f, -447.1f},
    {172.8f, -415.5f},
    {235.8f, -131.7f}
};
track_generate_barrier(control_points14, 7);

const Vector2f control_points15[] = {
    {-1242.8f, 473.3f},
    {-1214.8f, 319.4f},
    {-1126.2f, 216.8f},
    {-1005.0f, 188.8f},
    {-921.0f, 188.8f},
    {-874.4f, 296.1f},
    {-846.4f, 478.0f}
};
track_generate_barrier(control_points15, 7);

const Vector2f finish_pos = {-1046.9f, 468.7f};
track_set_finish(finish_pos);

const Vector2f spawn_pos[] = {
    {-287.8f, -65.9f},
    {-152.4f, -68.2f},
    {-5.4f, -70.6f},
    {132.4f, -70.6f}
};
track_set_spawns(spawn_pos);
}